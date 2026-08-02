#include <memory>
#include <iostream>


class Mass {
public:
	enum status {
		BLANK,
		PLAYER,
		ENEMY,
	};
private:
	status s_ = BLANK;
public:
	status getStatus() const { return s_; }
	void setStatus(status s) { s_ = s; }

	bool put(status s) {
		if (s_ != BLANK) return false;
		s_ = s;
		return true;
	}
};

class Board;

class AI {
public:
	AI() {}
	virtual ~AI() {}

	virtual bool think(Board& b) = 0;

public:
	enum type {
		TYPE_ORDERED = 0,
		TYPE_NEGA_MAX,
		TYPE_ALPHA_BETA,
		TYPE_NEGA_SCOUT,
		TYPE_EVALUATE_NEGASCOUT,
	};

	static AI* createAi(type type);
};

// 順番に打ってみる
class AI_ordered : public AI {
public:
	AI_ordered() {}
	~AI_ordered() {}

	bool think(Board& b);
};

class AI_nega_max : public AI {
private:
	int evaluate(Board& b, Mass::status next);
public:
	AI_nega_max() {}
	~AI_nega_max() {}

	int evaluate(Board& b, Mass::status current, int& best_x, int& best_y);

	bool think(Board& b);
};

class AI_alpha_beta :public AI {
private:
	int evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_alpha_beta() {}
	~AI_alpha_beta() {}

	bool think(Board& b);
};

class AI_nega_scout : public AI {
private:
	int evaluate(int limit, int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);
public:
	AI_nega_scout() {}
	~AI_nega_scout() {}

	bool think(Board& b);
};

class AI_evaluate_negascout : public AI {
private:
	static int cell_weight(int x, int y);
	static int line_score(Board& b, int y0, int x0, int dy, int dx, Mass::status player, Mass::status opponent);
	int evaluate_static(Board& b, Mass::status current);
	int evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y);
public:
	AI_evaluate_negascout() {}
	~AI_evaluate_negascout() {}

	bool think(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type) {
	case TYPE_NEGA_MAX:
		return new AI_nega_max();
	case TYPE_ALPHA_BETA:
		return new AI_alpha_beta();
	case TYPE_NEGA_SCOUT:
		return new AI_nega_scout();
	case TYPE_EVALUATE_NEGASCOUT:
		return new AI_evaluate_negascout();
	default: //TYPE_ORDERED
		return new AI_ordered();
	}

	return nullptr;
}

class Board
{
	friend class AI_ordered;

public:
	enum {
		BOARD_SIZE = 5,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

	enum WINNER {
		NOT_FINISED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};

public:
	Board() {
	}
	Board::WINNER calc_result() const
	{
		// 縦横斜めに同じキャラが入っているか検索
		// 横
		for (int y = 0; y < BOARD_SIZE; y++) {
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++) {
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++) {
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER) winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY) {
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++) {
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++) {
			for (int x = 0; x < BOARD_SIZE; x++) {
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISED;
			}
		}
		return DRAW;
	}

	bool put(int x, int y) {
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		return mass_[y][x].put(Mass::PLAYER);
	}

	void show() const {
		std::cout << "　　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << " " << x + 1 << "　";
		}
		std::cout << "\n　";
		for (int x = 0; x < BOARD_SIZE; x++) {
			std::cout << "＋－";
		}
		std::cout << "＋\n";
		for (int y = 0; y < BOARD_SIZE; y++) {
			std::cout << " " << char('a' + y);
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "｜";
				switch (mass_[y][x].getStatus()) {
				case Mass::PLAYER:
					std::cout << "〇";
					break;
				case Mass::ENEMY:
					std::cout << "× ";
					break;
				case Mass::BLANK:
					std::cout << "　";
					break;
				default:
//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "＋"; else
//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "＃"; else
					std::cout << "　";
				}
			}
			std::cout << "｜\n";
			std::cout << "　";
			for (int x = 0; x < BOARD_SIZE; x++) {
				std::cout << "＋－";
			}
			std::cout << "＋\n";
		}
	}
};

bool AI_ordered::think(Board& b)
{
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			if (b.mass_[y][x].put(Mass::ENEMY)) {
				return true;
			}
		}
	}
	return false;
}

//nega_max法
int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y) {
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = b.calc_result();
	if (r == current) return +10000; // 呼び出し側の勝ち
	if (r == next) return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	int score_max = -10001; // 打たないのは最悪

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current); // 次の手を打つ
			int dummy; // 最上位以外は打つわけではないのでダミーでごまかす
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK); // 手を戻す


			if (score_max < score) {
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}


bool AI_nega_max::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(b, Mass::ENEMY, best_x, best_y);

	if (best_x < 0) return false; // 打てる手無し

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

//alpha_beta法
int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	//死活判定
	int r = b.calc_result();
	if (r == current) return +10000; // 呼び出し側の勝ち
	if (r == next) return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	int score_max = -9999; //打たないで投了

	for (int y = 0; y < Board::BOARD_SIZE; y++){
		for (int x = 0; x < Board::BOARD_SIZE; x++){
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK); // 手を戻す

			if (beta < score)
			{
				return (score_max < score) ? score : score_max; // 最悪の値より悪い
			}
			if (score_max < score)
			{
				score_max = score;
				alpha = (alpha < score_max) ? score_max : alpha; // α値を更新
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}

// nega_scout法

int AI_nega_scout::evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y)
{
	if (limit-- == 0) return 0; // 深さ制限に達した、引き分けにしておく

	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	// 死活判定
	int r = board.calc_result();
	if (r == current) return +10000; // 呼び出し側の勝ち
	if (r == next) return -10000; // 呼び出し側の負け
	if (r == Board::DRAW) return 0; // 引き分け

	int a = alpha, b = beta;
	bool is_first = true;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current); // 次の手を打つ
			int dummy;
			int score = -evaluate(limit, -b, -a, board, next, dummy, dummy);
			if (a < score && score < beta && !is_first || 2 <= limit)
			{
				score = -evaluate(limit, -beta, -score, board, next, dummy, dummy);
			}
			is_first = false;

			m.setStatus(Mass::BLANK); // 手を戻す

			if (a < score) {
				a = score;
				best_x = x;
				best_y = y;
			}

			if (beta <= a) { // β刈り
				return a;
			}

			b = a + 1; //nullウィンドウの更新
		}
	}

	return a;
}

bool AI_alpha_beta::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false; //打てる手無し
	
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

bool AI_nega_scout::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(5, -10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)
		return false; //打てる手無し
	
	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

// その座標の重さ
int AI_evaluate_negascout::cell_weight(int x, int y)
{
	int weight = 2;								// 縦横で確定2本
	if (x == y) weight++;						// ＼の方向
	if (x + y == Board::BOARD_SIZE - 1) weight++;// ／の方向
	return weight;
}

// 線（ライン）の評価
int AI_evaluate_negascout::line_score(Board& b, int y0, int x0, int dy, int dx, Mass::status player, Mass::status opponent)
{
	int player_count = 0, opponent_count = 0;
	for (int i = 0; i < Board::BOARD_SIZE; i++) {
		Mass::status s = b.mass_[y0 + dy * i][x0 + dx * i].getStatus();
		if (s == player) player_count++;
		else if (s == opponent) opponent_count++;
	}

	if (player_count > 0 && opponent_count > 0) return 0; // 揃わない

	if (opponent_count > 0) {
		if (opponent_count == Board::BOARD_SIZE - 1) return -1000; // 相手が立直
		return -(opponent_count * opponent_count);
	}
	if (player_count > 0) {
		if (player_count == Board::BOARD_SIZE - 1) return +1000; // プレイヤーの立直
		return +(player_count * player_count);
	}
	return 0;
}

// 盤面全体を評価する
int AI_evaluate_negascout::evaluate_static(Board& b, Mass::status current)
{
	Mass::status opponent = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;
	int score = 0;

	// 縦横の評価
	for (int i = 0; i < Board::BOARD_SIZE; i++) {
		score += line_score(b, i, 0, 0, 1, current, opponent);
		score += line_score(b, 0, i, 1, 0, current, opponent);
	}
	// 斜め
	score += line_score(b, 0, 0, 1, 1, current, opponent);
	score += line_score(b, 0, Board::BOARD_SIZE - 1, 1, -1, current, opponent);

	// 重さ
	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass::status s = b.mass_[y][x].getStatus();
			if (s == Mass::BLANK) continue;
			int weight = cell_weight(x, y);
			score += (s == current) ? weight : -weight;
		}
	}

	return score;
}

// nega_scout法 + 盤面の重さと立直の検知
int AI_evaluate_negascout::evaluate(int limit, int alpha, int beta, Board& board, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	// 死活判定(優先)
	int r = board.calc_result();
	if (r == current) return +100000;
	if (r == next) return -100000;
	if (r == Board::DRAW) return 0;

	if (limit-- == 0) return evaluate_static(board, current); // 0ではなく盤面全体の評価を返す

	int a = alpha, b_ = beta;
	bool is_first = true;

	for (int y = 0; y < Board::BOARD_SIZE; y++) {
		for (int x = 0; x < Board::BOARD_SIZE; x++) {
			Mass& m = board.mass_[y][x];
			if (m.getStatus() != Mass::BLANK) continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(limit, -b_, -a, board, next, dummy, dummy);
			if (a < score && score < beta && !is_first)
			{
				score = -evaluate(limit, -beta, -score, board, next, dummy, dummy);
			}
			is_first = false;

			m.setStatus(Mass::BLANK);

			if (a < score) {
				a = score;
				best_x = x;
				best_y = y;
			}
			if (beta <= a) return a;
			b_ = a + 1;
		}
	}
	return a;
}

bool AI_evaluate_negascout::think(Board& b)
{
	int best_x = -1, best_y = -1;

	evaluate(3, -1000000, 1000000, b, Mass::ENEMY, best_x, best_y); // 一旦3に。増やしてもOK

	if (best_x < 0) return false; // 打てる手無し

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

class Game
{
private:
	const AI::type ai_type = AI::TYPE_EVALUATE_NEGASCOUT; // ここを変える

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISED;
	AI* pAI_ = nullptr;

public:
	Game() {
		pAI_ = AI::createAi(ai_type);
	}
	~Game() {
		delete pAI_;
	}

	bool put(int x, int y) {
		bool success = board_.put(x, y);
		if (success) winner_ = board_.calc_result();

		return success;
	}

	bool think() {
		bool success = pAI_->think(board_);
		if (success) winner_ = board_.calc_result();
		return success;
	}

	Board::WINNER is_finised() {
		return winner_;
	}

	void show() {
		board_.show();
	}
};




void show_start_message()
{
	std::cout << "========================" << std::endl;
	std::cout << "       GAME START       " << std::endl;
	std::cout << std::endl;
	std::cout << "input position likes 1 a" << std::endl;
	std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
	if (winner == Board::PLAYER) {
		std::cout << "You win!" << std::endl;
	}
	else if (winner == Board::ENEMY)
	{
		std::cout << "You lose..." << std::endl;
	}
	else {
		std::cout << "Draw" << std::endl;
	}
	std::cout << std::endl;
}

int main()
{
	for (;;) {// 無限ループ
		show_start_message();

		// initialize
		bool player_turn = true;
		std::shared_ptr<Game> game(new Game());

		while (1) {
			game->show();// 盤面表示

			// 勝利判定
			Board::WINNER winner = game->is_finised();
			if (winner) {
				show_end_message(winner);
				break;
			}

			if (player_turn) {
				// user input
				char col[1], row[1];
				do {
					std::cout << "? ";
					std::cin >> row >> col;
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else {
				// AI
				if (!game->think()) {
					show_end_message(Board::WINNER::PLAYER);// 投了
				}
				std::cout << std::endl;
			}
			// プレイヤーとAIの切り替え
			player_turn = !player_turn;
		}
	}

	return 0;
}