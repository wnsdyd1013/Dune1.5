// 1)�غ� intro outro O
// 2)Ŀ�� & ����â O
// 3)�߸� ���� X
// 4)���� 1�� ���� O
// 5)�ý��� �޽��� O

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
		// loop �� ������(��, TICK==10ms����) Ű �Է� Ȯ��
		KEY key = get_key();

		// Ű �Է��� ������ ó��
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// ����Ű ���� �Է�
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

		// ���� ������Ʈ ����
		sample_obj_move();

		// ȭ�� ���
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
		fflush(stdout); // ��� ���۸� ����� ��� ȭ�鿡 ǥ��
		Sleep(delay);  // ������ �ð� ���� (����: �и���)
	}
}

/* ================= subfunctions =================== */
void intro(void) {
	// Ÿ��Ʋ�� ���� ASCII ��Ʈ�� ���
	system("cls");
	printf("\n\n\n");

	// DUNE Ÿ��Ʋ �ִϸ��̼� (�� ���� ASCII ��Ʈ��)
	printf("  DDDD   U   U   N   N   EEEEE\n");
	printf("  D   D  U   U   NN  N   E    \n");
	printf("  D   D  U   U   N N N   EEEE \n");
	printf("  D   D  U   U   N  NN   E    \n");
	printf("  DDDD   UUUUU   N   N   EEEEE\n");

	// �ؽ�Ʈ Ÿ���� ȿ��
	print_typing_effect("\nDUNE 1.5\n", 150);  // Ÿ���� ȿ�� (delay: 150ms)

	// ��� ���߰� ȭ���� ���� �� ���ο� ȭ���� ���
	Sleep(1000);  // 1�� ���
	system("cls");

	system("cls"); // ȭ�� �ʱ�ȭ
	printf("��� ��, ������ ���۵˴ϴ�.\n");
	print_typing_effect("�غ� �Ǽ̳���? �� �����մϴ�!\n", 100);
	Sleep(1500);  // 1.5�� ���

	system("cls"); // ȭ�� �ʱ�ȭ
}

void outro(void) {
	system("cls"); // ȭ�� �ʱ�ȭ

	// �ƿ�Ʈ�� Ÿ���� ȿ�� (���ڰ� �ϳ��� ������ �ִϸ��̼�)
	printf("\n\n");
	print_typing_effect("������ �����ϴ� ��...\n", 150);  // Ÿ���� ȿ�� (delay: 150ms)

	// ��� ��� ��, ȭ���� ����� ���� �޽����� ���
	Sleep(1000);  // 1�� ���
	system("cls");

	// "exiting..." �޽����� �Բ� �ƿ�Ʈ�� �ִϸ��̼� �߰�
	print_typing_effect("Exiting...\n", 100);  // Ÿ���� ȿ��
	Sleep(500);  // �޽����� ������ ��µ� �� 0.5�� ���

	// ����
	exit(0);
}
void init(void) {
	// layer 0(map[0])�� ���� ����
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

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}

	// object sample
	map[1][obj.pos.row][obj.pos.column] = 'o';
}

#define NORMAL_MOVE_DISTANCE 1      // �⺻ �̵� �Ÿ�
#define FAST_MOVE_DISTANCE 3        // ���� Ŭ�� �� �̵� �Ÿ�
#define DOUBLE_CLICK_THRESHOLD_MS 200  // 200ms ���� ���� Ŭ�� �ν�

time_t last_key_time[4] = { 0, 0, 0, 0 };

// (�����ϴٸ�) ������ �������� Ŀ�� �̵�
void cursor_move(DIRECTION dir) {
	POSITION curr = cursor.current;

	// ���� �ð� ��������
	clock_t now = clock();  // `clock()`�� ����Ͽ� �и��� ���� �ð� ����

	// �̵� �Ÿ� ����
	int move_distance = NORMAL_MOVE_DISTANCE;
	if ((now - last_key_time[dir]) < DOUBLE_CLICK_THRESHOLD_MS * CLOCKS_PER_SEC / 1000) {
		move_distance = FAST_MOVE_DISTANCE;
	}

	// ���ο� ��ġ ���
	POSITION move_vector = dtop(dir);
	POSITION new_pos = { curr.row + move_vector.row * move_distance,
						 curr.column + move_vector.column * move_distance };

	// ��ȿ�� �˻�
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 &&
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {
		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}

	// ������ Ű �Է� �ð� ������Ʈ
	last_key_time[dir] = now;
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
	// ���� ��ġ�� �������� ���ؼ� �̵� ���� ����	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// ������ ����. ������ �ܼ��� ���� �ڸ��� �պ�
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright�� ������ ����
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft�� ������ ����
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// ������, ������ �Ÿ��� ���ؼ� �� �� �� ������ �̵�
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos�� ���� ����� �ʰ�, (������ ������)��ֹ��� �ε����� ������ ���� ��ġ�� �̵�
	// ������ �浹 �� �ƹ��͵� �� �ϴµ�, ���߿��� ��ֹ��� ���ذ��ų� ���� ������ �ϰų�... ���
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
		map[1][next_pos.row][next_pos.column] < 0) {

		return next_pos;
	}
	else {
		return obj.pos;  // ���ڸ�
	}
}

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) {
		// ���� �ð��� �� ����
		return;
	}

	// ������Ʈ(�ǹ�, ���� ��)�� layer1(map[1])�� ����
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}

// �ý��� �޽����� ����� �Լ�
void handle_spacebar() {
	POSITION curr_pos = cursor.current;

	// �÷��̾� �ǹ� (PLAYER_BASE) ������ �����̽��ٸ� ������ ��
	POSITION PLAYER_BASE[4] = { {15,1},{15,2},{16,1},{16,2} };
	for (int i = 0; i < 4; i++) {
		if (curr_pos.row == PLAYER_BASE[i].row && curr_pos.column == PLAYER_BASE[i].column) {
			display_system_message("H : �Ϻ����� ����\n");
			select_flag = true;
			return;
		}
	}

	// AI �ǹ� (AI_BASE) ������ �����̽��ٸ� ������ ��
	POSITION AI_BASE[4] = { {1,57},{1,58},{2,57},{2,58} };
	for (int i = 0; i < 4; i++) {
		if (curr_pos.row == AI_BASE[i].row && curr_pos.column == AI_BASE[i].column) {
			display_system_message("���� �ǹ��Դϴ�!!");
			return;
		}
	}

	// Rock ���� (ROCK_2X2) ������ �����̽��ٸ� ������ ��
	POSITION ROCK_2X2[2][4] = { {{4,15},{4,16},{5,15},{5,16}},
								{{10,37},{10,38},{11,37},{11,38}} };
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			if (curr_pos.row == ROCK_2X2[i][j].row && curr_pos.column == ROCK_2X2[i][j].column) {
				display_system_message("�����Դϴ�!!");
				return;
			}
		}
	}

	POSITION ROCK_1X1[3] = { {6,52},{9,20},{13,55} };

	for (int i = 0; i < 3; i++) {
		if (curr_pos.row == ROCK_1X1[i].row && curr_pos.column == ROCK_1X1[i].column) {
			display_system_message("�����Դϴ�!!");
			return;
		}
	}

	// �� ���� ���: �ƹ� �޽����� ǥ������ ����
	display_system_message("�縷 ����");
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
	gotoxy(system_message);  // �޽����� ����� ��ġ�� Ŀ�� �̵�

	int size = strlen(massage);

	// �޽����� message �迭�� �߰�
	if (message_count < 6) {
		// ���� �迭�� ���� ���� �ʾҴٸ�, ���ο� �޽����� �߰�
		memcpy(message[message_count], massage, size + 1);  // +1�� null ���Ṯ�ڵ� �����ϱ� ����
		message_count++;
	}
	else {
		// �迭�� ���� á����, �޽������� �� ĭ�� ���� �̵�
		for (int i = 0; i < 5; i++) {  // ������ �޽����� �����ϰ� �� ĭ�� �̵�
			memcpy(message[i], message[i + 1], strlen(message[i + 1]) + 1);
		}

		// ������ �޽��� �ڸ��� ���ο� �޽��� �߰�
		memcpy(message[5], massage, size + 1);  // +1�� null ���Ṯ�ڵ� �����ϱ� ����
	}

	set_color(COLOR_DEFAULT);

	// ������� ����� �޽������� ���
	for (int i = 0; i < message_count; i++) {
		printf("%s\n", message[i]);
	}
}