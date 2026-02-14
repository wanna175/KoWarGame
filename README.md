# 🪖 KoWarGame

---

## 📌 프로그램 개요

본 워게임은 **국방 지휘관 훈련용 Modeling & Simulation**에 🎮 게임적 요소를 추가하여 개발되었습니다.  

시뮬레이션 엔진은 **DEVS (Discrete Event System Specification)** 기반으로 설계되었으며,  
KAIST 김탁곤 교수님의 강의 및 논문을 참고하여 구현되었습니다.

---

## 🖥️ 클라이언트 프로그램

- 💻 **운영체제** : Windows OS  
- 🧑‍💻 **사용 언어**
  - C# (Unity)
  - C++ (NetworkModule.dll)

### ✨ 주요 기능
- 🗺️ 전장 상황도
- 🎯 모의 통제 도구
- 🧩 통합 UI 환경 제공

---

## 🔄 데이터 전달 처리기 (Data Transfer Processor)

- 🐧 **운영체제** : Linux (Ubuntu 22.04)  
- ⚙️ **사용 언어** : C++  

### 🎯 역할
- 클라이언트 ↔ 시뮬레이션 엔진 ↔ DB 서버 간 패킷 송수신 담당  

### 🌐 통신 구조

#### 🧠 Internal Server
- 📦 시뮬레이션 엔진과 IPC 통신  
  - Shared Memory 기반  
- ⚡ 하위 서버 Multiplexing 통신  
  - epoll 기반 이벤트 처리  

#### 🌍 External Server
- 🎮 클라이언트 통신  
- 🛰️ 상위 서버 통신  
- ⚡ epoll 기반 Multiplexing 처리  

---

## 🧪 시뮬레이션 엔진

- 🐧 **운영체제** : Linux (Ubuntu 22.04)  
- ⚙️ **사용 언어** : C++  

### 🚀 주요 기능

#### 📅 이벤트 관리
- 이산 사건(Event) 기반 시뮬레이션 수행  
- 이벤트 큐 기반 상태 변화 처리  

#### ⏱️ 시간 동기화
- 실제 시간 (Real-Time) ↔ 시뮬레이션 시간 동기화  

#### 🧬 모델 관리
- 모델 간 상호작용을 이벤트 기반으로 관리  

#### 📐 이산 사건 시스템
- DEVS 기반 시뮬레이션 구조 구현  

---

## 🗄️ DB Server

- 🛢️ **DBMS** : MySQL  

---

## 📄 상세 문서

👉 **[KoWarGame ReadMe PDF 보기](https://github.com/user-attachments/files/25315295/PDF.KoWarGame.ReadMe.pdf)**


## 📌 시스템 아키텍처 개요


