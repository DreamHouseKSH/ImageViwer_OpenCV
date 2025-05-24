#!/bin/bash

# 현재 디렉토리 상태 확인
git status

# 모든 변경 사항 스테이징
git add .

# 현재 시간 기반 커밋 메시지 생성
current_time=$(date +"%Y%m%d-%H%M")
commit_message="자동 커밋: $current_time"

# 커밋 실행
git commit -m "$commit_message"

# 원격 저장소로 푸쉬
git push origin $(git rev-parse --abbrev-ref HEAD)

# 완료 메시지 출력
echo "커밋 및 푸쉬가 완료되었습니다."
