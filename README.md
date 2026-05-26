# ⚔ProjectARPG
> Unreal Engine 5 & C++ 기반의 세키로 스타일 액션 RPG 포트폴리오 프로젝트  
> 상태 기반 전투 시스템과 AI 전투 흐름 구현에 집중하여 제작했습니다.

<br>

# 🎮 Project Overview

Project ARPG는 Unreal Engine 5와 C++ 기반으로 개발한  
3D 액션 RPG 포트폴리오 프로젝트입니다.

단순 기능 구현이 아닌,

- 전투 상태 전이 구조
- 패링 및 체간 시스템
- AI 전투 판단 로직
- 귀불 체크포인트 시스템

등 실제 액션 RPG에서 사용되는 핵심 전투 흐름을 직접 설계하고 구현하는 것을 목표로 제작했습니다.

<br>

# 🔥 Core Features

## ⚔ Combat System

### 패링 / 막기 / 피격 처리
- 공격 속성 기반 피격 반응 처리
- 패링 성공 시 적 경직 및 체간 누적
- 상태 기반 전투 흐름 관리

![Parry](Docs/GIF/parry.gif)

---

### 체간(Posture) 시스템
- 공격 및 방어 시 체간 누적
- 체간 브레이크 상태 전이
- 인살 가능 상태 처리

![Posture](Docs/GIF/posture.gif)

---

### 인살(Execution) 시스템
- 체간 브레이크 이후 인살 가능
- 애니메이션 연동 실행 처리
- 상태 고정 및 사망 처리 구현

![Execution](Docs/GIF/execution.gif)

<br>
