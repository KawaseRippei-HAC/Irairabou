#define _CRT_SECURE_NO_WARNINGS
#include <GSgameLight.h>

const int ScreenWidth = 640;
const int ScreenHeight = 480;

class MyGame : public gslib::Game {

public:

public:
	// コンストラクタ
	MyGame() :gslib::Game{ ScreenWidth,ScreenHeight }{}

private:

	// プレイヤー構造体	
	struct stPlayer
	{
		float x; // x座標
		float y; // y座標
		float vx; // x方向速度
		float vy; // y方向速度
	};

	//カメラ
	struct Camera
	{
		// カメラの位置
		// 画面左上のワールド票を表す
		float x;
		float y;
	};

	// ゲームの状態の種別の定義
	enum State
	{
		Play,  // プレイ中
		Dead,  // 死んだ
		Goal,  // ゴールした
	};

	static const int MapChipSize = 32;  // 1マスの幅・高さ(ピクセル数)
	static const int MapWidth = 80;     // マップの幅(マスの数)
	static const int MapHeight = 60;    // マップの高さ(マスの数)
	int map[MapHeight][MapWidth] = { -1 };      // マップ情報を格納するための2次元配列
	stPlayer stPlayer;
	State state;                        // ゲームの状態
	static const int PlayerSize = 16;   // プレイヤーの幅・高さ(ピクセル数)
	float Acceleration = 0.3f;          // 加速力
	float Damp = 0.98f;                 // 速度の減衰率
	const int FinalLevel = 2;           // 最終面の番号
	int level;                          // 現在の面(1から始まる)
	Camera camera;                      // カメラ

	// 画像のバンドル
	enum
	{
		Mapchip,
		Player,
	};

	// マップチップの定義
	enum MapChip_kind
	{
		MapFloor = 0,     // 床
		MapWall,          // 壁
		MapStart,       // スタート地点
		MapGoal,          // ゴール地点
	};


	// ステージを初期状態にする	
	void Reset()
	{
		LoadMap(level);         // レベルに応じたcsvを読み込む
		stPlayer.vx = 0;
		stPlayer.vy = 0;
		SetPlayerPosition();
		state = Play;
		camera.x = 0;
		camera.y = 0;
	}

	// 開始
	void start()override {
		gsLoadTexture(Mapchip, "Image/mapchip.png");
		gsLoadTexture(Player, "Image/pacchi_16.png");
		level = 1;
		Reset();
		gsFontParameter(GS_FONT_NORMAL, 32, "MSゴシック"); // フォント変更
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	// 更新
	void update(float delta_time)override {
		// プレイ状態の時
		if (state == Play)
		{
			// プレイヤー更新処理
			{
				// キー入力に応じて加速する
				if (gsGetKeyState(GKEY_LEFT))
				{
					stPlayer.vx -= Acceleration;
				}
				if (gsGetKeyState(GKEY_RIGHT))
				{
					stPlayer.vx += Acceleration;
				}
				if (gsGetKeyState(GKEY_UP))
				{
					stPlayer.vy -= Acceleration;
				}
				if (gsGetKeyState(GKEY_DOWN))
				{
					stPlayer.vy += Acceleration;
				}

				// 速度の分だけ移動する
				stPlayer.x += stPlayer.vx;
				stPlayer.y += stPlayer.vy;

				// 減速
				stPlayer.vx *= Damp;
				stPlayer.vy *= Damp;
			}
			// プレイヤーが壁にぶつかってたら死亡
			if (PlayerIntersectsWall())
			{
				state = Dead;
			}
			// ゴールに入ってたらゴール状態にする
			else if (IsPlayerInGoal())
			{
				state = Goal;
			}
		}
		// プレイヤー死亡時
		else if (state == Dead)
		{
			// ボタンを押したらリスタート
			if (gsGetKeyState(GKEY_Z))
			{
				Reset();
			}
		}
		// ゴール時
		else if (state == Goal)
		{
			// まだ最終面じゃなくてボタンを押した場合
			if (level < FinalLevel && gsGetKeyState(GKEY_Z))
			{
				// 次の面へ
				level += 1;
				Reset();
			}
		}
		// プレイヤーが画面の真ん中に来るように、カメラ位置を更新
		LookAt(stPlayer.x, stPlayer.y);
	}
	// 指定したワールド座標のマップデータを退却する
	int GetMapData(float worldX, float worldY)
	{
		// ワールド座標をマップ配列のインデックスに変換
		int mapX = (int)(worldX / MapChipSize);
		int mapY = (int)(worldY / MapChipSize);
		return map[mapY][mapX];
	}

	// 指定したワールド座標が壁かどうかを判定する
	// 壁ならtrue,壁でなければfalseを返却する
	bool IsWall(float worldX, float worldY)
	{
		return GetMapData(worldX, worldY) == MapWall;
	}


	// プレイヤーが壁に重なっているか？
	// プレイヤーの左上、右上、左下、右下の4点を調べ,
	// いずれか一か所でも壁であれば、壁に重なっていると判定
	bool PlayerIntersectsWall()
	{
		return
			IsWall(stPlayer.x + 3, stPlayer.y + 3) ||
			IsWall(stPlayer.x - 3 + PlayerSize, stPlayer.y + 3) ||
			IsWall(stPlayer.x + 3, stPlayer.y + PlayerSize - 3) ||
			IsWall(stPlayer.x - 3 + PlayerSize, stPlayer.y + PlayerSize - 3);
	}

	// プレイヤーがゴールに入っているか？
	bool IsPlayerInGoal()
	{
		return GetMapData(stPlayer.x, stPlayer.y) == MapGoal;
	}

	// 描画
	void draw()override {
		// マップの描画
		// 画面内のマップのみ描画するようにする
		int left = (int)(camera.x / MapChipSize);
		int top = (int)(camera.y / MapChipSize);
		int right = (int)((camera.x + ScreenWidth - 1) / MapChipSize);
		int bottom = (int)((camera.y + ScreenHeight - 1) / MapChipSize);
		if (left < 0)left = 0;
		if (top < 0)top = 0;
		if (right >= MapWidth)right = MapWidth - 1;
		if (bottom >= MapHeight)bottom = MapHeight - 1;

		for (int y = top; y < MapHeight; y++)
		{
			for (int x = left; x < MapWidth; x++)
			{
				int mapchipID = map[y][x];
				if (mapchipID < 0)continue;
				GSvector2 pos = { (GSfloat)x * MapChipSize - camera.x,
					(GSfloat)y * MapChipSize - camera.y };
				GSrect src = { (GSfloat)mapchipID * MapChipSize,0,
					(GSfloat)mapchipID * MapChipSize + MapChipSize,0 + MapChipSize };
				gsDrawSprite2D(Mapchip, &pos, &src, NULL, NULL, NULL, 0.0f);

			}
		}
		GSvector2 playerpos = { stPlayer.x - camera.x,stPlayer.y - camera.y };
		GSrect playersrc = { 0,0,PlayerSize,PlayerSize };
		gsDrawSprite2D(Player, &playerpos, &playersrc, NULL, NULL, NULL, 0.0f);

		// 死亡時
		if (state == Dead)
		{
			DrawString("PUSH Z KEY TO RETRY.", 140, 230);
		}
		// ゴール状態
		else if (state == Goal)
		{
			// まだ最終面じゃない
			if (level < FinalLevel)
			{
				DrawString("PUSH Z KEY TO NEXT LEVEL.",140, 230);
			}
			// 最終面	
			else
			{
				DrawString("GAME CLEAR!!", 200, 230);
			}
		}

	}

	// プレイヤーの位置をスタート地点にする

		// プレイヤーの位置をスタート地点にする
	void SetPlayerPosition()
	{
		for (int y = 0; y < MapHeight; y++)
		{
			for (int x = 0; x < MapWidth; x++)
			{
				// MapStartの所にプレイヤー配置
				if (map[y][x] == MapStart)
				{
					stPlayer.x = x * MapChipSize + PlayerSize / 2;
					stPlayer.y = y * MapChipSize + PlayerSize / 2;
					return; // ループを打ち切って関数終了
				}
			}
		}
	}

	// 終了
	void end()override {

	}

	// マップcsvの読み込み処理
	void LoadMap(int level)
	{
		// マップデータを
		
		FILE* fp; // FILE型構造体
		char fname[256];
		snprintf(fname, 256, "Map/map%d.csv", level);
		char line[MapWidth * 4];
		fp = fopen(fname, "r"); // ファイルを開く。失敗するとNULLを返す

		if (fp == NULL)
		{
			printf("%s file not open!\n", "Map/map1.csv");
			return;
		}

		int y = 0;
		while (fgets(line, sizeof(line), fp) != NULL)
		{
			printf("%s", line);
			char* p1; // char型ポインタワーク
			int x = 0;
			p1 = strtok(line, ","); // 1個目の部分文字列取得
			// 分割する文字列がなくなるまで繰り返します。
			while (p1 != NULL)
			{
				map[y][x] = atoi(p1);
				p1 = strtok(NULL, ","); // 2個目以降の部分文字列取得

				x++;
			}
			y++;
		}
		fclose(fp); // ファイルを閉じる
	}
	// 指定されたワールド座標が画面の中心に来るように、カメラの位置を変更する
	void LookAt(float targetX, float targetY)
	{
		// カメラの移動範囲を制限するための定数
		const int MinCameraX = 0;
		const int MinCameraY = 0;
		const int MaxCameraX = MapChipSize * MapWidth - ScreenWidth;
		const int MaxCameraY = MapChipSize * MapHeight - ScreenHeight;
		
		// カメラの俊敏さ。0～1で指定。小さいほどのろ～っとし、大きいほどシュッと動く
		const float Agility = 0.07f;
		// 目的地に徐々に近づける
		camera.x = camera.x + (targetX - ScreenWidth / 2 - camera.x) 
			* Agility;
		camera.y = camera.y + (targetY - ScreenHeight / 2 - camera.y)
			* Agility;

		// マップデータが存在しない場所を移さないように、カメラの移動範囲を制限する
		if (camera.x < MinCameraX) camera.x = MinCameraX;
		if (camera.x > MaxCameraX) camera.x = MaxCameraX;
		if (camera.y < MinCameraY) camera.y = MinCameraY;
		if (camera.y > MaxCameraY) camera.y = MaxCameraY;
		
	}
};

// メイン関数
int main() {
	return MyGame().run();
}