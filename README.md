# KoWarGame
0. 프로그램 개요

이 워게임은 국방 지휘관 훈련용 modeling &amp; simulation을 약간의 게임적 요소를 첨가하여 개발되었습니다. 시뮬레이션 엔진은 DEVS(Discrete Event System Specification)을 기반으로  만들어졌으며 한국과학기술원(KAIST) 김탁곤 교수님의 강의, 논문을 참고하였습니다. 

2. 소프트웨어(SW) 구조
   
<img width="616" height="404" alt="image" src="https://github.com/user-attachments/assets/9a54b0d9-84eb-4d37-9ea0-65a3c759471a" />

■ 클라이언트 프로그램:
⦁ 운영체제 : windows os
⦁ 사용언어 : C#(unity), C++(NetworkModule.dll)
⦁ 모의통제도구, 전장상황도 등을 통합하여 구현

■ 데이터 전달 처리기
⦁ 운영체제 : linux os(ubuntu 22.04)
⦁ 사용언어 : C++
⦁ 클라이언트와 시뮬레이션 엔진, DB서버 간 패킷 송수신 담당
⦁ internal server: 시뮬레이션 엔진과 ipc(공유메모리), 하위서버와 멀티플렉싱(epoll)                    방식으로 통신
⦁ external server: 클라이언트, 상위서버와 멀티플렉싱(epoll) 방식으로 통신

■ 시뮬레이션 엔진
⦁ 운영체제 : linux os(ubuntu 22.04)
⦁ 사용언어 : C++
⦁ 이벤트 관리: 이산 사건 이벤트를 관리하고 결과를 도출
⦁ 시간 동기화: 실제 시간과 시뮬레이션을 동기화 
⦁ 모델 관리: 정의된 모델의 상호작용을 이벤트로 관리
⦁ 이산 사건 시스템: DEVS를 기반으로 시뮬레이션을 구현 

■ DB server
⦁ MySQL 서버 사용 

2.소프트웨어(SW) 실행 화면
2.1 클라이언트 UI 프로그램
<img width="540" height="311" alt="image" src="https://github.com/user-attachments/assets/c5fb61ca-2551-410c-9b7b-ba80a5e27fe9" />
                              [그림 1] 클라이언트 로그인 화면
<img width="571" height="330" alt="image" src="https://github.com/user-attachments/assets/77a2f73c-5b70-47de-9ec5-15f9325a66fc" />
                              [그림 2] 클라이언트 시뮬레이션 컨트롤 탭 화면
<img width="570" height="321" alt="image" src="https://github.com/user-attachments/assets/de85d898-f540-4eef-aa9c-898e5484a3fe" />
                              [그림 3] 클라이언트 전장 상황도 화면
<img width="595" height="338" alt="image" src="https://github.com/user-attachments/assets/479d6534-d8b7-4537-8995-ff37e8442003" />
                              [그림 4] 클라이언트 전장 상황도 유닛 명령 화면
<img width="596" height="335" alt="image" src="https://github.com/user-attachments/assets/2a9725ef-5d5a-47e6-8b38-3f86e575eacf" />
                              [그림 5] 클라이언트 전장 상황도 유닛 피격 화면
2.2 데이터 전달 처리 서버 및 시뮬레이션 엔진 서버
<img width="487" height="343" alt="image" src="https://github.com/user-attachments/assets/c4454571-a4f0-4b09-ae09-74f8c9e3599d" />
                              [그림 6] 데이터 전달 처리 실행 화면
<img width="487" height="343" alt="image" src="https://github.com/user-attachments/assets/f6661e8f-a2b5-48b0-93ed-7cfb977174f0" />
                              [그림 6] 데이터 전달 처리 실행 화면
<img width="510" height="300" alt="image" src="https://github.com/user-attachments/assets/65377a66-2513-47a3-a4ab-e430b325fb08" />
                              [그림 7] 시뮬레이션 엔진 서버 실행 화면
<img width="459" height="252" alt="image" src="https://github.com/user-attachments/assets/82492ab5-28f6-4096-9ea0-d01ec8f201e7" />
                              [그림 8] 시뮬레이션 엔진 초기 시나리오 수신
                                                      
