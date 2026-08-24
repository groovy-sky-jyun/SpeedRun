# SpeedRun
지형을 실시간으로 스캔해 상황에 맞는 파쿠르 액션을 자동으로 실행하는 3인칭 이동 시스템.

| 장르 | 인원 | 기간 | 엔진 |
|---|---|---|---|
| 3인칭 파쿠르 액션 | 1인 | 2025.11 — 2026.04 | Unreal Engine 5.7 |

---
## 주요 기능
- **실시간 지형 스캔** — 전방 `SphereTrace`로 파쿠르 대상을 찾고 렛지 · 상단면 · 착지면의 위치와 치수를 측정

- **상황 기반 액션 선택** — 등록된 액션이 환경 수치로 각자 점수를 매기고, 최고점 액션이 실행

- **파쿠르 액션 3종** — Vault(넘어가기) · Mantle(올라서기) · Hang(매달리기)

- **매달리기 전용 이동 모드** — 커스텀 이동 모드에서 벽면 좌우 이동(Shimmy)과 양손 IK 처리

- **데이터 주도 애니메이션** — Chooser Table이 높이 · 깊이 · 속도 범위로 몽타주를 선택

- **파쿠르 블록** — 레벨에 배치하고 크기만 조절하면 난간 데이터와 치수 표시가 자동 갱신

- **상태별 입력 전환** — 매달린 상태에서는 같은 키가 다른 동작으로 매핑

- **경사각 속도 제어** — 오르막에서 경사각에 비례해 이동 속도를 감쇠

  

---
## 조작법

| 키 | 동작 |
|---|---|
| `W` `A` `S` `D` | 이동 |
| `마우스` | 시점 |
| `Space` | 점프 · 파쿠르 (상황에 따라 Vault / Mantle / Hang 자동 선택) |
| `C` | 슬라이드 · 웅크리기 · 매달린 상태에서 내려오기 |
| `Q` | 대시 |
| `F` | 상호작용 |

  

> [!NOTE]
> 매달린 상태에서는 같은 키라도 동작이 달라진다.
>`Space`는 올라서기, `C`는 내려오기, `A` `D`는 좌우 이동(Shimmy)이 됩니다.

  

---

## 기술 스택

| | |
|---|---|
| 엔진 | Unreal Engine 5.7 |
| 언어 | C++ · Blueprint (데이터 · 애니메이션 조립) |
| 플러그인 | Chooser · Motion Warping · Landmass |
| 개발 툴 | Visual Studio · Git |
| 리소스 | Mixamo |

---

  

## 프로젝트 구조

```

Source/SpeedRun/
├─ Component/
│  ├─ ParkourComponent            환경 스캔 · 액션 평가 · 실행
│  └─ ParkourMovementComponent    커스텀 이동 모드 · 경사각 속도 제어
├─ DA/
│  ├─ ParkourActionBase           액션 공통 인터페이스
│  ├─ ParkourAction_Vault         넘어가기
│  ├─ ParkourAction_Mantle        올라서기
│  ├─ ParkourAction_Hang          매달리기
│  └─ DA_*TraceOption             트레이스 규격 데이터
├─ ParkourBlock                   레벨 배치용 액터 · 난간 스플라인 자동 생성
├─ PlayerAnimInstance             Shimmy 속도 · 양손 IK
└─ SpeedRunPlayerController       상태별 InputMappingContext 전환

```

액션을 추가하려면 `UParkourActionBase`를 상속한 클래스를 만들고, `BP_Player`의 `RegisteredActions` 배열에 등록하면 됩니다.


---
## 빌드

Unreal Engine 5.7과 Visual Studio 2022가 필요합니다.

```

1. SpeedRun.uproject 우클릭 → Generate Visual Studio project files

2. SpeedRun.sln 열기 → Development Editor / Win64 로 빌드

3. SpeedRun.uproject 실행

```

  

