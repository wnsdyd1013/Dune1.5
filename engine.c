// 1)준비 intro outro O
// 2)커서 & 상태창 O
// 3)중립 유닛 X
// 4)유닛 1기 생산 O
// 5)시스템 메시지 O

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);
void handle_spacebar();
void produce_harvester(void);
void print_message(char massage[]);
void print_typing_effect(const char* text, int delay);

int harvester_count = 0;
HARVESTER harvesters[20];

/* ================= control =================== */
int sys_clock = 0;		// system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };


/* ================= game data =================== */
char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = {
	.spice = 100,
	.spice_max = 100,
	.population = 0,
	.population_max = 0
};

OBJECT_SAMPLE obj = {
	.pos = {1, 1},
	.dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
	.repr = 'o',
	.speed = 300,
	.next_move_time = 300
};

bool select_flag = false;

/* ================= main() =================== */
int main(void) {
	srand((unsigned int)time(NULL));

	init();
	intro();
	display(resource, map, cursor);

	while (1) {
		// loop 돌 때마다(즉, TICK==10ms마다) 키 입력 확인
		KEY key = get_key();

		// 키 입력이 있으면 처리
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// 방향키 외의 입력
			switch (key) {
			case k_quit: outro();
			case k_space: handle_spacebar();  break;
			case k_escape: esc_system_message("                            "); select_flag = false; break;
			case k_h:
				if (select_flag) {
					produce_harvester();
				}
				break;
			case k_none:
			case k_undef:
			default: break;
			}
		}

		// 샘플 오브젝트 동작
		sample_obj_move();

		// 화면 출력
		display(resource, map, cursor);
		for (int i = 0; i < harvester_count; i++) {
			display_harvester(harvesters[i]);
		}
		Sleep(TICK);
		sys_clock += 10;
	}
}

void print_typing_effect(const char* text, int delay) {
	for (int i = 0; text[i] != '\0'; i++) {
		putchar(text[i]);
		fflush(stdout); // 출력 버퍼를 비워서 즉시 화면에 표시
		Sleep(delay);  // 딜레이 시간 설정 (단위: 밀리초)
	}
}

/* ================= subfunctions =================== */
void intro(void) {
	// 타이틀을 멋진 ASCII 아트로 출력
	system("cls");
	printf("\n\n\n");

	// DUNE 타이틀 애니메이션 (더 멋진 ASCII 아트로)
	printf("  DDDD   U   U   N   N   EEEEE\n");
	printf("  D   D  U   U   NN  N   E    \n");
	printf("  D   D  U   U   N N N   EEEE \n");
	printf("  D   D  U   U   N  NN   E    \n");
	printf("  DDDD   UUUUU   N   N   EEEEE\n");

	// 텍스트 타이핑 효과
	print_typing_effect("\nDUNE 1.5\n", 150);  // 타이핑 효과 (delay: 150ms)

	// 잠시 멈추고 화면을 지운 뒤 새로운 화면을 출력
	Sleep(1000);  // 1초 대기
	system("cls");

	system("cls"); // 화면 초기화
	printf("잠시 후, 게임이 시작됩니다.\n");
	print_typing_effect("준비 되셨나요? 곧 시작합니다!\n", 100);
	Sleep(1500);  // 1.5초 대기

	system("cls"); // 화면 초기화
}

void outro(void) {
	system("cls"); // 화면 초기화

	// 아웃트로 타이핑 효과 (문자가 하나씩 나오는 애니메이션)
	printf("\n\n");
	print_typing_effect("게임을 종료하는 중...\n", 150);  // 타이핑 효과 (delay: 150ms)

	// 잠시 대기 후, 화면을 지우고 종료 메시지를 출력
	Sleep(1000);  // 1초 대기
	system("cls");

	// "exiting..." 메시지와 함께 아웃트로 애니메이션 추가
	print_typing_effect("Exiting...\n", 100);  // 타이핑 효과
	Sleep(500);  // 메시지가 완전히 출력된 후 0.5초 대기

	// 종료
	exit(0);
}
void init(void) {
	// layer 0(map[0])에 지형 생성
	for (int j = 0; j < MAP_WIDTH; j++) {
		map[0][0][j] = '#';
		map[0][MAP_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		map[0][i][0] = '#';
		map[0][i][MAP_WIDTH - 1] = '#';
		for (int j = 1; j < MAP_WIDTH - 1; j++) {
			map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])은 비워 두기(-1로 채움)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}

	// object sample
	map[1][obj.pos.row][obj.pos.column] = 'o';
}

#define NORMAL_MOVE_DISTANCE 1      // 기본 이동 거리
#define FAST_MOVE_DISTANCE 3        // 더블 클릭 시 이동 거리
#define DOUBLE_CLICK_THRESHOLD_MS 200  // 200ms 내에 더블 클릭 인식

time_t last_key_time[4] = { 0, 0, 0, 0 };

// (가능하다면) 지정한 방향으로 커서 이동
void cursor_move(DIRECTION dir) {
	POSITION curr = cursor.current;

	// 현재 시간 가져오기
	clock_t now = clock();  // `clock()`을 사용하여 밀리초 단위 시간 측정

	// 이동 거리 결정
	int move_distance = NORMAL_MOVE_DISTANCE;
	if ((now - last_key_time[dir]) < DOUBLE_CLICK_THRESHOLD_MS * CLOCKS_PER_SEC / 1000) {
		move_distance = FAST_MOVE_DISTANCE;
	}

	// 새로운 위치 계산
	POSITION move_vector = dtop(dir);
	POSITION new_pos = { curr.row + move_vector.row * move_distance,
						 curr.column + move_vector.column * move_distance };

	// 유효성 검사
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 &&
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {
		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}

	// 마지막 키 입력 시간 업데이트
	last_key_time[dir] = now;
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
	// 현재 위치와 목적지를 비교해서 이동 방향 결정	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// 목적지 도착. 지금은 단순히 원래 자리로 왕복
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright로 목적지 설정
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft로 목적지 설정
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// 가로축, 세로축 거리를 비교해서 더 먼 쪽 축으로 이동
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos가 맵을 벗어나지 않고, (지금은 없지만)장애물에 부딪히지 않으면 다음 위치로 이동
	// 지금은 충돌 시 아무것도 안 하는데, 나중에는 장애물을 피해가거나 적과 전투를 하거나... 등등
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
		map[1][next_pos.row][next_pos.column] < 0) {

		return next_pos;
	}
	else {
		return obj.pos;  // 제자리
	}
}

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) {
		// 아직 시간이 안 됐음
		return;
	}

	// 오브젝트(건물, 유닛 등)은 layer1(map[1])에 저장
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}

// 시스템 메시지를 출력할 함수
void handle_spacebar() {
	POSITION curr_pos = cursor.current;

	// 플레이어 건물 (PLAYER_BASE) 위에서 스페이스바를 눌렀을 때
	POSITION PLAYER_BASE[4] = { {15,1},{15,2},{16,1},{16,2} };
	for (int i = 0; i < 4; i++) {
		if (curr_pos.row == PLAYER_BASE[i].row && curr_pos.column == PLAYER_BASE[i].column) {
			display_system_message("H : 하베스터 생산\n");
			select_flag = true;
			return;
		}
	}

	// AI 건물 (AI_BASE) 위에서 스페이스바를 눌렀을 때
	POSITION AI_BASE[4] = { {1,57},{1,58},{2,57},{2,58} };
	for (int i = 0; i < 4; i++) {
		if (curr_pos.row == AI_BASE[i].row && curr_pos.column == AI_BASE[i].column) {
			display_system_message("적의 건물입니다!!");
			return;
		}
	}

	// Rock 지역 (ROCK_2X2) 위에서 스페이스바를 눌렀을 때
	POSITION ROCK_2X2[2][4] = { {{4,15},{4,16},{5,15},{5,16}},
								{{10,37},{10,38},{11,37},{11,38}} };
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			if (curr_pos.row == ROCK_2X2[i][j].row && curr_pos.column == ROCK_2X2[i][j].column) {
				display_system_message("바위입니다!!");
				return;
			}
		}
	}

	POSITION ROCK_1X1[3] = { {6,52},{9,20},{13,55} };

	for (int i = 0; i < 3; i++) {
		if (curr_pos.row == ROCK_1X1[i].row && curr_pos.column == ROCK_1X1[i].column) {
			display_system_message("바위입니다!!");
			return;
		}
	}

	// 그 외의 경우: 아무 메시지도 표시하지 않음
	display_system_message("사막 지형");
}

void produce_harvester(void) {
	POSITION system_message = { 20,1 };
	if (resource.spice > 5) {
		HARVESTER har = {
	.pos = {15,1},
	.dest = {12,1},
	.repr = 'H',
	.speed = 1000,
	.next_move_time = 1000
		};
		harvesters[harvester_count] = har;
		harvester_count++;

		
		
		print_message("A new harvester ready                ");

		resource.spice -= 5;
	}
	else {
		gotoxy(system_message);

		print_message("Not enough spice                  ");
	}
}

int message_x = 20;
char message[6][100] = { 0 };
int message_count = 0;

void print_message(char massage[]) {
	POSITION system_message = { message_x, 0 };
	gotoxy(system_message);  // 메시지를 출력할 위치로 커서 이동

	int size = strlen(massage);

	// 메시지를 message 배열에 추가
	if (message_count < 6) {
		// 아직 배열이 가득 차지 않았다면, 새로운 메시지를 추가
		memcpy(message[message_count], massage, size + 1);  // +1은 null 종료문자도 복사하기 위해
		message_count++;
	}
	else {
		// 배열이 가득 찼으면, 메시지들을 한 칸씩 위로 이동
		for (int i = 0; i < 5; i++) {  // 마지막 메시지를 제외하고 한 칸씩 이동
			memcpy(message[i], message[i + 1], strlen(message[i + 1]) + 1);
		}

		// 마지막 메시지 자리에 새로운 메시지 추가
		memcpy(message[5], massage, size + 1);  // +1은 null 종료문자도 복사하기 위해
	}

	set_color(COLOR_DEFAULT);

	// 현재까지 저장된 메시지들을 출력
	for (int i = 0; i < message_count; i++) {
		printf("%s\n", message[i]);
	}
}