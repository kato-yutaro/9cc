#ifndef __9CC_H__
typedef enum {
    TK_RESERVED, //記号
    TK_NUM, //整数
    TK_EOF, //入力の終わり
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len; //トークンの長さ
};

//抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ, // ==
    ND_NE, // !=
    ND_LT, // <
    ND_LE, // <=
    ND_NUM
} NodeKind;

typedef struct Node Node;

//抽象構文木のノード
struct Node {
    NodeKind kind; 
    Node *lhs; //左子ノード
    Node *rhs; //右子ノード
    int val;
};

void error_at(char *loc, char * fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();
Node* new_node(NodeKind kind, Node *lhs, Node* rhs);
Node* new_node_num(int val);
Node* expr();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

void gen(Node *node);

extern Token *token; //現在注目しているトークン

extern char *user_input; //入力プログラム

#endif