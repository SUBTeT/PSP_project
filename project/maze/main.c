#include <dxlibp.h>
#include "COLORS.H"
#include <pspaudiocodec.h>
#include <pspaudio.h>
#include <psputility.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspuser.h>
#include <pspgu.h>
#include <pspctrl.h>
/***************************************************************************
* 定義
***************************************************************************/
#define WIDE 480
#define VERTICAL 272

#define MAZE_ROW 5 //行数
#define MAZE_COLUMN 5 //列数
#define FONTSIZE_ 12
#define LEVEL 3

/***************************************************************************
* enum, typedef
***************************************************************************/

//迷路の一ブロック
enum MazeKind {PATH, WALL, START, GOAL};    //ブロックの種類

enum MazeFlag {TRUE_, FALSE_}; //ブロック判明の有無

enum MazeDirection {UP, DOWN, LEFT, RIGHT, Invalid}; //方向

enum Level {level1, level2, level3};

typedef struct
{
  enum MazeKind kind;            //種類(道、壁、スタート、ゴール)
  enum MazeFlag flag;            //ブロックが判明しているかどうか
}MazeBlock;

typedef struct
{
	int row;
	int column;
} MazePlayer;

/*****************************************************************************
*  関数の登録
*****************************************************************************/
int  initton(void);
void ScreenSettings(void);
int  main(int argc, char *argp[]);
void action(int);
void MazeMenu();
int SelectLevel();
void ShowFailed();

/*****************************************************************************
*  グローバル変数
*****************************************************************************/

/*****************************************************************************
*  もじゅ〜る・いんふぉ  /  メインコードはユーザモード
*****************************************************************************/
PSP_MODULE_INFO("MAZE", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(16*1024); // 新規追加
PSP_MAIN_THREAD_STACK_SIZE_KB(512); // 新規追加

/*****************************************************************************
*  main
*****************************************************************************/
int main(int argc, char *argp[])
{
	if(DxLib_Init() == -1)
    ShowFailed();

  int level;
  int MHandle;
  int SHandle;

	if(initton() == 0){
    InitSoundMem();
    SetCreateSoundDataType(DX_SOUNDDATATYPE_FILE);

    if((MHandle = LoadSoundMem("ms0:/PSP/GAME/maze/a.mp3")) == -1)
      ShowFailed();
    SetPanSoundMem(0, MHandle);
    PlaySoundMem(MHandle, DX_PLAYTYPE_LOOP, TRUE);
    MazeMenu();
    level = SelectLevel();
    StopSoundMem(MHandle);

    if((SHandle = LoadSoundMem("ms0:/PSP/GAME/maze/b.mp3")) == -1)
      ShowFailed();
    SetPanSoundMem( 0, SHandle);
    PlaySoundMem(SHandle, DX_PLAYTYPE_LOOP, TRUE);
		action(level);  // 初期化成功(正常時）の時だけ action()関数を実行
    StopSoundMem(SHandle);
  }


	DxLib_End();
	return 0;
}

/*****************************************************************************
*  グラフィック画面の設定などの関数
*****************************************************************************/
void ScreenSettings()
{
	SetDisplayFormat(DXP_FMT_8888);//画面のピクセルフォーマットを32Bit色モードに
	SetWaitVSyncFlag(TRUE);    // 画面更新の際、垂直同期待ちをしてから行なう様に
	SetDrawArea(0,0,480,272);         // 描画可能領域を全領域に設定
	SetDrawMode(DX_DRAWMODE_NEAREST); // 描画モードを高速なNEARESTにセットする
	// ↓ 描画の際のブレンドモードを、標準モードに
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetDrawBright(255,255,255);       // 描画する色の輝度を最大に
	SetGraphMode(480, 272, DXP_FMT_8888); // フル画面サイズ ＆ 32Bit色モード
	ClearDrawScreen();                // 描画先のグラフィックスをクリアする
}

/*****************************************************************************
*  初期化専門
*****************************************************************************/
int initton(void)
{
	ScreenSettings(); // グラフィック画面の設定など

    // エッジ無しノーマル文字にする(IntraFont)
	ChangeFontType(DX_FONTTYPE_NORMAL);
	SetFontSize(FONTSIZE_);

    // 音楽再生はしません

	return 0;
}

/*****************************************************************************
*  アクション
*****************************************************************************/
void PadWait()
{
	do {
		ProcessMessage();
	} while(CheckHitKeyAll() == 0);
}

void ShowFailed()
{
  pspDebugScreenInit();
	pspDebugScreenClear();
	pspDebugScreenPrintf("Failed!\n");
	sceKernelDelayThread(5*1000*1000);
	sceKernelExitGame();
}

int SelectLevel()
{
  int level;
  int pad_states;

  LoadGraphScreen(0, 0, "ms0:/PSP/GAME/maze/select.png", TRUE);
  ScreenFlip();
  ClearDrawScreen();

  while(1)
  {
    ProcessMessage();
    pad_states = GetInputState();
    if(pad_states == DXP_INPUT_SQUARE){
      level = level1;
      return level;
    }
    if(pad_states == DXP_INPUT_TRIANGLE){
      level = level2;
      return level;
    }
    if(pad_states == DXP_INPUT_CIRCLE){
      level = level3;
      return level;
    }
  }
}

void MazeMenu()
{
  int pad_states;

  LoadGraphScreen(0, 0, "ms0:/PSP/GAME/maze/menu.png", TRUE);
  ScreenFlip();

  while(1)
  {
    ProcessMessage();
    pad_states = GetInputState();
    if(pad_states == DXP_INPUT_START)
      break;
    if(pad_states == DXP_INPUT_SELECT)
      sceKernelExitGame();
  }
}

int MazePlayerInit(int *playerRow, int *playerColumn, MazeBlock maze[MAZE_ROW][MAZE_COLUMN])
{
	int i, j;
	for(i = 0; i < MAZE_ROW; i++)
	{
		for(j = 0; j < MAZE_COLUMN; j++)
		{
			if(maze[i][j].kind == START)
			{
				*playerRow = i;
				*playerColumn = j;
				return 0; //関数終了
			}
		}
	}

	//スタート地点がなければ、プレイヤーを設定できずここにくる
	pspDebugScreenInit();
	pspDebugScreenClear();
	pspDebugScreenPrintf("CANT FIND START.");
	sceKernelDelayThread(1*1000*1000);
	return -1;
}

void MazePlayerMove(int *playerRow, int *playerColumn, MazeBlock maze[MAZE_ROW][MAZE_COLUMN])
{
	int pad_states;

	ProcessMessage();
	pad_states = GetInputState();

	switch (pad_states) {
		case DXP_INPUT_UP: //上移動
		{
			if(*playerRow - 1 >= 0) //迷路の範囲外でないことを確認
			{
				maze[*playerRow - 1][*playerColumn].flag = TRUE_; //ブロックの種類が判明

				if(maze[*playerRow - 1][*playerColumn].kind != WALL) //壁かどうか確認
				{
					*playerRow -= 1; //移動
				}
				else
				{
					DrawFormatString(300, 250, WHITE, "壁です。");
				}
			}
			else
			{
				DrawFormatString(300, 250, WHITE, "範囲外です。");
			}
		}
		break;

		case DXP_INPUT_DOWN: //した移動
		{
			if(*playerRow + 1 < MAZE_ROW)
			{
				maze[*playerRow + 1][*playerColumn].flag = TRUE_;

				if(maze[*playerRow + 1][*playerColumn].kind != WALL)
				{
					*playerRow += 1;
				}
				else
				{
					DrawFormatString(300, 250, WHITE, "壁です。");
				}
			}
			else
			{
				DrawFormatString(300, 250, WHITE, "範囲外です。");
			}
		}
		break;

		case DXP_INPUT_LEFT: //左移動
		{
			if(*playerColumn - 1 >= 0)
			{
				maze[*playerRow][*playerColumn - 1].flag = TRUE_;

				if(maze[*playerRow][*playerColumn - 1].kind != WALL)
				{
					*playerColumn -= 1;
				}
				else
				{
					DrawFormatString(300, 250, WHITE, "壁です。");
				}
			}
			else
			{
				DrawFormatString(300, 250, WHITE, "範囲外です。");
			}
		}
		break;

		case DXP_INPUT_RIGHT: //右移動
		{
			if(*playerColumn + 1 < MAZE_COLUMN)
			{
				maze[*playerRow][*playerColumn + 1].flag = TRUE_;

				if(maze[*playerRow][*playerColumn + 1].kind != WALL)
				{
					*playerColumn += 1;
				}
				else
				{
					DrawFormatString(300, 250, WHITE, "壁です。");
				}
			}
			else
			{
        DrawFormatString(300, 250, WHITE, "範囲外です。");
			}
		}
		break;

		case DXP_INPUT_START:
			sceKernelExitGame();
	}
}

int MazeGoalCheck(int playerRow, int playerColumn, MazeBlock maze[MAZE_ROW][MAZE_COLUMN])
{
	if(maze[playerRow][playerColumn].kind == GOAL)
	{
		return 1;
	}
	return 0;
}

void MazeDraw(int playerRow, int playerColumn, MazeBlock maze[MAZE_ROW][MAZE_COLUMN])
{
	int i, j;

	for(i = 0; i < MAZE_ROW; i++) //行
	{
		for(j = 0; j < MAZE_COLUMN; j++) //列
		{
			if(i == playerRow && j == playerColumn) //プレイヤー位置
			{
				DrawFormatString(FONTSIZE_*j, FONTSIZE_*i, WHITE, "P");
			}
			else if(maze[i][j].flag == FALSE_) //ブロックが判明していない
			{
				DrawFormatString(FONTSIZE_*j, FONTSIZE_*i, WHITE, "？");
			}
			else
			{
				switch (maze[i][j].kind) {
					case WALL:
						DrawFormatString(FONTSIZE_*j, FONTSIZE_*i, WHITE, "■"); //壁
					case GOAL:
						DrawFormatString(FONTSIZE_*j, FONTSIZE_*i, WHITE, "G"); //ゴール
					default:
						DrawFormatString(FONTSIZE_*j, FONTSIZE_*i, WHITE, " "); //その他
				}
			}
		}

	}
	ScreenFlip();
	PadWait();
}

void action(int level)
{
	//プレイヤー
	MazePlayer player;
  int start, end;

  //迷路
	MazeBlock maze[LEVEL][MAZE_ROW][MAZE_COLUMN] =
	{
		{//level1
      {{START, TRUE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}},
		  {{WALL, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}},
		  {{PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}},
		  {{WALL, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {WALL, FALSE_}, {WALL, FALSE_}},
		  {{PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {GOAL, TRUE_}},
    },
    {//level2
      {{START, TRUE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}},
		  {{WALL, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}},
		  {{PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}},
		  {{WALL, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {WALL, FALSE_}, {WALL, FALSE_}},
		  {{PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {GOAL, TRUE_}},
    },
    {//level3
      {{START, TRUE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}},
		  {{WALL, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}},
		  {{PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {PATH, FALSE_}},
		  {{WALL, FALSE_}, {PATH, FALSE_}, {WALL, FALSE_}, {WALL, FALSE_}, {WALL, FALSE_}},
		  {{PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {PATH, FALSE_}, {GOAL, TRUE_}},
    },
	};
	//プレイヤー初期化
	if(MazePlayerInit(&player.row, &player.column, maze[level]) == -1)
	{
		//関数MazePlayerInitが-1を返す時
		//初期化に失敗しているのでこの時点でプログラムを終了する
		sceKernelExitGame();
	}

  start = GetNowCount();
	while(MazeGoalCheck(player.row, player.column, maze[level]) != 1)
	{
		//迷路表示
		ScreenFlip();
    ClearDrawScreen();
		DrawBox(0, 0, WIDE, VERTICAL, BLACK, TRUE);
		MazeDraw(player.row, player.column, maze[level]);
		//プレイヤー移動
		MazePlayerMove(&player.row, &player.column, maze[level]);
    ScreenFlip();
	}

	//迷路最終結果表示
	ScreenFlip();
  ClearDrawScreen();
  end = GetNowCount() - start;
	DrawBox(0, 0, WIDE, VERTICAL, BLACK, TRUE);
	DrawFormatString(150, 250, WHITE, "ゴールです! %f[s]かかりました。", (float)end / 1000);
	MazeDraw(player.row, player.column, maze[level]);

}
