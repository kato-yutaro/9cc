#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

void error_at(char *loc, char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

void expect(char *op) {
    if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }

    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize() {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char* p = user_input;
    while(*p) {
        if(isspace(*p)) {
            p++;
            continue;
        }

        if(strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if(isdigit(*p)) {
            char *tmp = p;
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            cur->len = p - tmp;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

Node* new_node(NodeKind kind, Node *lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr() {
    return equality();
}

Node *equality() {
    Node *node = relational();
    for(;;) {
        if(consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();
    for(;;) {
        if(consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if(consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if(consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if(consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node* add() {
    Node *node = mul();
    for(;;) {
        if(consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();
    for(;;) {
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    for(;;) {
        if(consume("+")) {
            return primary();
        }
        if(consume("-")) {
            return new_node(ND_SUB, new_node_num(0), primary());
            //単項-を0-に置き換える
        }
        return primary();
    } 
}

Node *primary() {
    if(consume("(")) {
        Node* node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}