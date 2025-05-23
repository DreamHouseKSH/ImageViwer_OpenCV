#include "airphoto_viewer/core/tile_cache.h"
#include <QDebug>
#include <chrono>
#include <numeric>

namespace airphoto_viewer {
namespace core {

// ThreadPool Implementation
TileCache::ThreadPool::ThreadPool(size_t numThreads) 
    : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this] { 
                        return stop || !tasks.empty(); 
                    });
                    
                    if (stop && tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                
                task();
            }
        });
    }
}

TileCache::ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    
    condition.notify_all();
    
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

template<class F, class... Args>
auto TileCache::ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type> {
    
    using return_type = typename std::result_of<F(Args...)>::type;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        
        tasks.emplace([task]() { (*task)(); });
    }
    
    condition.notify_one();
    return res;
}

// TileCache Implementation
TileCache::TileCache(size_t maxMemoryMB)
    : maxMemory(maxMemoryMB * 1024 * 1024)
    , currentMemoryUsage(0)
    , tileSize(256) {
    
    // Use number of CPU cores for the thread pool
    unsigned int numThreads = std::thread::hardware_concurrency();
    threadPool = std::make_unique<ThreadPool>(numThreads > 0 ? numThreads : 4);
}

TileCache::~TileCache() {
    clear();
}

bool TileCache::setSourceImage(const std::string& imagePath) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    
    // Clear existing cache
    clear();
    
    // Try to load the image
    sourceImage = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    
    if (sourceImage.empty()) {
        qWarning() << "Failed to load image:" << QString::fromStdString(imagePath);
        return false;
    }
    
    qDebug() << "Loaded image:" << QString::fromStdString(imagePath)
             << "size:" << sourceImage.cols << "x" << sourceImage.rows
             << "channels:" << sourceImage.channels()
             << "type:" << sourceImage.type();
    
    return true;
}

TilePtr TileCache::getTile(int level, int x, int y) {
    TileKey key{level, x, y};
    
    // Process any completed tasks first
    processCompletedTasks();
    
    // Check if the tile is already in the cache
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        auto it = cache.find(key);
        
        if (it != cache.end()) {
            // Update last access time
            it->second.lastAccess = std::chrono::steady_clock::now();
            return it->second.tile;
        }
        
        // If not in cache, create a new tile and start loading it
        TilePtr tile = std::make_shared<Tile>(level, x, y, tileSize);
        
        // Calculate the size of this tile (approximate)
        size_t tileSizeBytes = tileSize * tileSize * sourceImage.channels() * 
                              (sourceImage.depth() == CV_8U ? 1 : 4);
        
        // Add to cache with a loading future
        auto& entry = cache[key];
        entry.tile = tile;
        entry.size = tileSizeBytes;
        entry.lastAccess = std::chrono::steady_clock::now();
        
        // Start loading the tile asynchronously
        entry.loadingFuture = threadPool->enqueue([this, key]() {
            // This runs in a worker thread
            TileKey k = key;  // Make a copy for the lambda capture
            
            // Calculate the source rectangle for this tile
            int scale = 1 << key.level;  // 2^level
            int tileScaledSize = tileSize * scale;
            int srcX = key.x * tileScaledSize;
            int srcY = key.y * tileScaledSize;
            
            // Ensure we don't go out of bounds
            srcX = std::min(srcX, sourceImage.cols - 1);
            srcY = std::min(srcY, sourceImage.rows - 1);
            
            int width = std::min(tileScaledSize, sourceImage.cols - srcX);
            int height = std::min(tileScaledSize, sourceImage.rows - srcY);
            
            if (width <= 0 || height <= 0) {
                // Tile is completely outside the image
                return;
            }
            
            // Extract the region of interest
            cv::Rect roi(srcX, srcY, width, height);
            cv::Mat tileData;
            
            if (scale > 1) {
                // For higher levels, we need to downsample the image
                cv::Mat roiImage(sourceImage, roi);
                cv::resize(roiImage, tileData, cv::Size(tileSize, tileSize), 
                          0, 0, cv::INTER_AREA);
            } else {
                tileData = sourceImage(roi).clone();
            }
            
            // Update the tile data in the main thread
            std::lock_guard<std::mutex> lock(completedTasksMutex);
            completedTasks.push([this, k, tileData = std::move(tileData)]() {
                auto it = cache.find(k);
                if (it != cache.end()) {
                    it->second.tile->loadFromMat(tileData, 
                        cv::Rect(0, 0, tileData.cols, tileData.rows));
                }
            });
        });
        
        currentMemoryUsage += tileSizeBytes;
        
        // Evict old tiles if we're using too much memory
        if (currentMemoryUsage > maxMemory) {
            evictIfNeeded();
        }
        
        return tile;
    }
}

size_t TileCache::tileCount() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return cache.size();
}

size_t TileCache::memoryUsage() const {
    std::lock_guard<std::mutex> lock(cacheMutex);
    return currentMemoryUsage;
}

void TileCache::clear() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    cache.clear();
    currentMemoryUsage = 0;
    
    // Clear any pending tasks
    std::queue<std::function<void()>> empty;
    std::swap(completedTasks, empty);
}

QSize TileCache::sourceSize() const {
    if (sourceImage.empty()) {
        return QSize(0, 0);
    }
    return QSize(sourceImage.cols, sourceImage.rows);
}

void TileCache::evictIfNeeded() {
    // Simple LRU eviction
    while (currentMemoryUsage > maxMemory / 2 && !cache.empty()) {
        auto oldest = cache.begin();
        auto oldestTime = oldest->second.lastAccess;
        
        for (auto it = cache.begin(); it != cache.end(); ++it) {
            if (it->second.lastAccess < oldestTime) {
                oldest = it;
                oldestTime = it->second.lastAccess;
            }
        }
        
        currentMemoryUsage -= oldest->second.size;
        cache.erase(oldest);
    }
}

void TileCache::processCompletedTasks() {
    std::queue<std::function<void()>> tasks;
    
    {
        std::lock_guard<std::mutex> lock(completedTasksMutex);
        if (completedTasks.empty()) {
            return;
        }
        std::swap(tasks, completedTasks);
    }
    
    while (!tasks.empty()) {
        tasks.front()();
        tasks.pop();
    }
}

} // namespace core
} // namespace airphoto_viewer
