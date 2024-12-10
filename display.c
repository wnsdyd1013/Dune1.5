/*
*  display.c:
* 화면에 게임 정보를 출력
* 맵, 커서, 시스템 메시지, 정보창, 자원 상태 등등
* io.c에 있는 함수들을 사용함
*/

#include "display.h"
#include "io.h"

// 출력할 내용들의 좌상단(topleft) 좌표
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };


char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void print_building();
void job();

void display(
	RESOURCE resource,
	char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
	CURSOR cursor)
{
	display_resource(resource);
	display_map(map);
	display_cursor(cursor);
	job();
	// display_system_message()
	// display_object_info()
	// display_commands()
	// ...
}

void job(void) {
	POSITION system_massage = { 0,62 };
	gotoxy(system_massage);
	set_color(COLOR_DEFAULT);
	printf("상태 & 명령");
}

void display_resource(RESOURCE resource) {
	set_color(COLOR_RESOURCE);
	gotoxy(resource_pos);
	printf("spice = %d/%d, population=%d/%d\n",
		resource.spice, resource.spice_max,
		resource.population, resource.population_max
	);
}

// subfunction of draw_map()
void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]) {
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			for (int k = 0; k < N_LAYER; k++) {
				if (src[k][i][j] >= 0) {
					dest[i][j] = src[k][i][j];
				}
			}
		}
	}
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	project(map, backbuf);
	print_building();

	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (frontbuf[i][j] != backbuf[i][j]) {
				POSITION pos = { i, j };
				printc(padd(map_pos, pos), backbuf[i][j], COLOR_DEFAULT);
			}
			frontbuf[i][j] = backbuf[i][j];
		}
	}
}

// frontbuf[][]에서 커서 위치의 문자를 색만 바꿔서 그대로 다시 출력
void display_cursor(CURSOR cursor) {
	POSITION prev = cursor.previous;
	POSITION curr = cursor.current;

	char ch = frontbuf[prev.row][prev.column];
	printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

	ch = frontbuf[curr.row][curr.column];
	printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}

void print_building(void) {
	POSITION PLAYER_BASE[4] = { {16,1},{16,2},{17,1},{17,2} };
	for (int i = 0; i < 4; i++) {
		set_color(159);
		gotoxy(PLAYER_BASE[i]);
		printf("B");
	}

	POSITION AI_BASE[4] = { {2,57},{2,58},{3,57},{3,58} };
	for (int i = 0; i < 4; i++) {
		set_color(207);
		gotoxy(AI_BASE[i]);
		printf("B");
	}

	POSITION PLATE[8] = { {16,3},{16,4},{17,3},{17,4},
						{2,55},{2,56},{3,55},{3,56} };
	for (int i = 0; i < 8; i++) {
		set_color(143);
		gotoxy(PLATE[i]);
		printf("P");
	}

	POSITION PLAYER_SPICE = { 12,1 };
	POSITION AI_SPICE = { 7,58 };
	set_color(111);
	gotoxy(PLAYER_SPICE);
	printf("5");
	gotoxy(AI_SPICE);
	printf("5");

	POSITION ROCK_2X2[2][4] = { {{5,15},{5,16},{6,15},{6,16}},
							{{11,37},{11,38},{12,37},{12,38}} };
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			set_color(127);
			gotoxy(ROCK_2X2[i][j]);
			printf("R");
		}
	}

	POSITION ROCK_1X1[3] = { {7,52},{10,20},{14,55} };
	for (int i = 0; i < 3; i++) {
		set_color(127);
		gotoxy(ROCK_1X1[i]);
		printf("R");
	}
}

// 새로 추가된 함수: 시스템 메시지를 출력하는 함수
void display_system_message(const char* message) {
	// 메시지가 출력될 위치 (여기서는 맵 오른쪽 끝)
	POSITION message_pos = { 1, MAP_WIDTH + 2 };

	// 기존 메시지를 지우기 위해 공백으로 덮어쓰기
	set_color(COLOR_DEFAULT);  // 기본 색상으로 변경 (원하는 색상으로 변경 가능)
	gotoxy(message_pos);  // 메시지 출력 위치로 커서 이동
	printf("                                       ");  // 기존 메시지 지우기 위해 공백을 덮어 씁니다.

	set_color(COLOR_RESOURCE);  // 메시지 색상 설정
	gotoxy((POSITION) { 1, MAP_WIDTH + 2 });  // 맵 오른쪽 끝에 메시지 출력
	printf("%s", message);  // 메시지 출력
}

void esc_system_message(const char* message) {
	// 메시지가 출력될 위치 (여기서는 맵 오른쪽 끝)
	POSITION message_pos = { 1, MAP_WIDTH + 2 };

	// 기존 메시지를 지우기 위해 공백으로 덮어쓰기
	set_color(COLOR_DEFAULT);  // 기본 색상으로 변경 (원하는 색상으로 변경 가능)
	gotoxy(message_pos);  // 메시지 출력 위치로 커서 이동
	printf("%s", message);  // 기존 메시지 지우기 위해 공백을 덮어 씁니다.
}

void display_harvester(HARVESTER har) {
	gotoxy(har.pos);
	set_color(159);
	printf("%c", har.repr);
}