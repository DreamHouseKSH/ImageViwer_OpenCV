#ifndef AIRPHOTO_VIEWER_TILE_CACHE_H
#define AIRPHOTO_VIEWER_TILE_CACHE_H

#include "tile.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include <future>
#include <queue>

namespace airphoto_viewer {
namespace core {

/**
 * @brief Manages a cache of tiles for efficient rendering of large images.
 * 
 * This class handles loading, caching, and providing access to tiles.
 * It uses a thread pool for asynchronous loading of tiles.
 */
class TileCache {
public:
    /**
     * @brief Construct a new Tile Cache object
     * 
     * @param maxMemoryMB Maximum memory to use for the cache in MB
     */
    explicit TileCache(size_t maxMemoryMB = 512);
    ~TileCache();

    // Prevent copying
    TileCache(const TileCache&) = delete;
    TileCache& operator=(const TileCache&) = delete;

    /**
     * @brief Set the source image for the tile cache
     * 
     * @param imagePath Path to the source image file
     * @return true if the image was loaded successfully
     */
    bool setSourceImage(const std::string& imagePath);
    
    /**
     * @brief Get a tile from the cache, loading it if necessary
     * 
     * @param level The level of detail
     * @param x The x-coordinate of the tile
     * @param y The y-coordinate of the tile
     * @return TilePtr A shared pointer to the tile (may be loading asynchronously)
     */
    TilePtr getTile(int level, int x, int y);
    
    /**
     * @brief Get the total number of tiles in the cache
     * 
     * @return size_t The number of tiles
     */
    size_t tileCount() const;
    
    /**
     * @brief Get the total memory usage of the cache in bytes
     * 
     * @return size_t Memory usage in bytes
     */
    size_t memoryUsage() const;
    
    /**
     * @brief Clear the cache, releasing all tiles
     */
    void clear();
    
    /**
     * @brief Get the size of the source image
     * 
     * @return QSize The size of the source image, or (0,0) if no image is loaded
     */
    QSize sourceSize() const;

private:
    // Key for the tile cache
    struct TileKey {
        int level;
        int x;
        int y;
        
        bool operator==(const TileKey& other) const {
            return level == other.level && x == other.x && y == other.y;
        }
        
        struct Hash {
            size_t operator()(const TileKey& k) const {
                return std::hash<int>()(k.level) ^ 
                       (std::hash<int>()(k.x) << 1) ^ 
                       (std::hash<int>()(k.y) << 2);
            }
        };
    };
    
    // Cache entry with additional metadata
    struct CacheEntry {
        TilePtr tile;
        size_t size;  // Size in bytes
        std::chrono::steady_clock::time_point lastAccess;
        std::future<void> loadingFuture;
    };
    
    // Thread pool for async loading
    class ThreadPool {
    public:
        explicit ThreadPool(size_t numThreads);
        ~ThreadPool();
        
        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) 
            -> std::future<typename std::result_of<F(Args...)>::type>;
            
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        bool stop;
    };
    
    // Private methods
    void loadTileAsync(const TileKey& key);
    void evictIfNeeded();
    void processCompletedTasks();
    
    // Member variables
    std::unique_ptr<ThreadPool> threadPool;
    std::unordered_map<TileKey, CacheEntry, TileKey::Hash> cache;
    mutable std::mutex cacheMutex;  // Marked as mutable for const operations
    size_t maxMemory;
    size_t currentMemoryUsage;
    cv::Mat sourceImage;
    int tileSize;
    
    // Queue of completed tasks to be processed
    std::queue<std::function<void()>> completedTasks;
    std::mutex completedTasksMutex;
};

} // namespace core
} // namespace airphoto_viewer

#endif // AIRPHOTO_VIEWER_TILE_CACHE_H
