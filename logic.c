#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>


/*******************************************************************************




*******************************************************************************/





//用于调试的log宏
//#define DEBUG 1
#ifdef DEBUG
#define s_log(format,...) printf(format,##__VA_ARGS__)
#else
#define s_log(format,...)
#endif


//最大变元数
#define MAX 10


typedef int bool;

//表达式中的符号
const wchar_t s_and = L'∧';
const wchar_t s_or = L'∨';
const wchar_t s_not = L'¬';
const wchar_t s_imp = L'→';
const wchar_t s_eql = L'↔';
const wchar_t s_xor = L'⊕';
const wchar_t s_df = L'#';
const wchar_t s_n = L'%';
const wchar_t s_lb = L'(';
const wchar_t s_rb = L')';
const wchar_t s_comma = L',';
const wchar_t s_0 = L'0';
const wchar_t s_1 = L'1';
const wchar_t s_9 = L'9';
const wchar_t s_a = L'a';
const wchar_t s_z = L'z';
const wchar_t s_A = L'A';
const wchar_t s_Z = L'Z';
const wchar_t s_b = L' ';

struct var_node {
        wchar_t name[10];
};

struct fun_node {
        wchar_t name[10];
        wchar_t result[1024];
        int number;
};

struct string_node {
        wchar_t str[1024];
        wchar_t result[5];
};

//存放不等价公式的集合
struct string_node set16[16];
int begin_set = 0;
int end_set = 0;
int len_set = 0;

FILE *fp_input;
FILE *fp_output;
//运算符的优先级
#define flag_lb 0
#define flag_fun 1
#define flag_not 2
#define flag_and_or 3
#define flag_xor_imp_eql 4
#define flag_rb 5

//表达式存在的变元列表
struct var_node v_table[MAX];
//表达式存在的函数（新定义的逻辑连接词）列表
struct fun_node f_table[MAX];
//变元的个数
int nv = 0;
//函数（新定义的逻辑连接词）的个数
int nf = 0;

bool b_table[11][1024];

//输入缓冲区，用于存放表达式
wchar_t buff[1024];
//当前缓冲区中表达式的总的长度
int length = 0;

//运算符和运算数栈
wchar_t stack_op[1024];
bool stack_value[1024];
int p_op = 0;
int p_value = 0;

void push_op(wchar_t wt);
void push_value(bool b);
wchar_t pop_op();
bool pop_value();
int check_buff();
int check_brace(int i);
int check_name(int i);
int check_define();
int check_parameter(int i, int index);
void output_error(int i);
void add_v(int b, int e);
void add_f(int b, int e);
int is_in_vt(int b, int e);
int is_in_ft(int b, int e);
void copy_name(wchar_t *name, int b, int e);
bool is_same_name(wchar_t *name, int b, int e);
void output_var();
void output_fun();
void init_table();
void output_table();
void output_value();
int calculate(int flag);
int calculate_fun(wchar_t wt);
int calculate_buff();
int calculate_end();
void output_true_false();


void init_set16();
void get_set();
int add_set();
void output_set();
void output_not_imp();
int is_in_set(wchar_t *t);
int generate_str();
int swap_var(int p);
int copy_str(wchar_t *l, wchar_t *r, int len);
void add_to_set(int len, wchar_t *t);
bool is_same_v(wchar_t *l, wchar_t *r, int len);

static inline bool is_blank(wchar_t wc);
static inline bool is_define(wchar_t wc);

int main()
{
        int r = 0;
        int i = 0;
        fp_input = fopen("input.txt", "rb");
        fp_output = fopen("output.txt", "wb");
        if (fp_input == NULL || fp_output == NULL) {
                wprintf(L"Open files error!");
                fputws(L"Open files error!",fp_output);
                exit(-1);
        }
        setlocale(LC_ALL, "");
        while ((fgetws(buff, 1024, fp_input))>0) {
                nv = 0;
                nf = 0;
                i = 0;
                r = 0;
                wprintf(L"%ls\n", buff);
                fputws(buff,fp_output);
                length = wcslen(buff);

                //取一行中的第一个非空白字符，若为新联结词定义则求完全集，否则做表达式求值
                while (is_blank(buff[i]))
                        ++i;
                if (i >= length) {
                        continue;
                }

                if (is_define(buff[i])) {
                        wprintf(L"判断完全集：\n");
                        fputws(L"判断完全集：\r\n",fp_output);
                        //检查新联结词的定义
                        r = check_define();
                        if (r < 0) {
                                s_log("r = %d\n", r);
                                wprintf(L"\r\n\n\n");
                                fputws(L"\r\n\n\n",fp_output);
                                continue;
                        }

                        //求不等价公式
                        get_set();
                } else {
                        wprintf(L"表达式求值：\n");
                        fputws(L"表达式求值：\n",fp_output);
                        //检查新联结词的定义
                        r = check_define();
                        if (r < 0) {
                                s_log("r = %d\n", r);
                                wprintf(L"\n\n\n");
                                fputws(L"\r\n\n\n",fp_output);
                                continue;
                        }
                        //语法检查
                        r = check_buff();
                        if (r < 0) {
                                s_log("r = %d\n", r);
                                wprintf(L"\n\n\n");
                                fputws(L"\r\n\n\n",fp_output);
                                continue;
                        }
                        init_table();

                        //计算缓冲区的表达式
                        r = calculate_buff();
                        if (r < 0) {
                                s_log("r = %d\n", r);
                                wprintf(L"\n\n\n");
                                fputws(L"\r\n\n\n",fp_output);
                                continue;
                        }
                        //output_value();
                        //输出表达式计算的结果
                        output_true_false();
                }


                wprintf(L"\n\n\n");
                fputws(L"\r\n\n\n",fp_output);
        }

        fclose(fp_input);
        fclose(fp_output);
        return 0;
}





static inline bool f_and(bool l, bool r)
{
        return (l&r);
}
static inline bool f_or(bool l, bool r)
{
        return (l|r);
}
static inline bool f_imp(bool l, bool r)
{
        return !(l&!r);
}
static inline bool f_eql(bool l, bool r)
{
        return ((l&r)|(!(l|r)));
}
static inline bool f_xor(bool l, bool r)
{
        return (l^r);
}
static inline bool f_not(bool p)
{
        return !p;
}
static inline bool is_not(wchar_t wc)
{
        return (wc == s_not);
}
static inline bool is_op(wchar_t wc)
{
        return (wc == s_and || wc == s_or || wc == s_xor || wc == s_imp || wc == s_eql);
}
static inline bool is_bool(wchar_t wc)
{
        return (wc == s_0 || wc == s_1);
}
static inline bool is_number(wchar_t wc)
{
        return (wc >= s_0 && wc <= s_9);
}
static inline bool is_alphabet(wchar_t wc)
{
        return ((wc >= s_a && wc <= s_z)||(wc >= s_A && wc <= s_Z));
}
static inline bool is_lbrace(wchar_t wc)
{
        return (wc == s_lb);
}
static inline bool is_rbrace(wchar_t wc)
{
        return (wc == s_rb);
}
static inline bool is_blank(wchar_t wc)
{
        return (wc == s_b || wc == 13  || wc == 10 || wc == 65279 || wc == 0);
}
static inline bool is_note(wchar_t wc)
{
        return (wc == s_n);
}
static inline bool is_define(wchar_t wc)
{
        return (wc == s_df);
}
static inline bool is_comma(wchar_t wc)
{
        return (wc == s_comma);
}
static inline bool is_and_or(wchar_t wc)
{
        return (wc == s_and || wc == s_or);
}
static inline bool is_xor_eql_imp(wchar_t wc)
{
        return (wc == s_xor || wc == s_imp || wc == s_eql);
}

//对全局缓冲区buff数组进行语法检查
int check_buff()
{
        int flag = 0;
        int r = 0;
        int i = 0;
        int p = 0;
        while (i < length) {
                while (is_blank(buff[i]))
                        ++i;
                if (i >= length) {
                        return 0;
                }
                if (is_note(buff[i])) {
                        i = length;
                        //s_log("check_buff: note!\n");
                } else if (is_define(buff[i])) {
                        i = length;
                        s_log("check_buff: define!\n");
                } else if (is_not(buff[i])) {
                        s_log("check_buff: not!\n");
                        ++i;

                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_alphabet(buff[i]) || is_bool(buff[i]) || is_lbrace(buff[i]) || is_not(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }
                } else if (is_op(buff[i]) ) {
                        if (!flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 0;
                        ++i;
                        s_log("check_buff: operational character!\n");

                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_alphabet(buff[i]) || is_bool(buff[i]) || is_lbrace(buff[i]) || is_not(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }
                } else if (is_lbrace(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        s_log("check_buff: left brace!\n");
                        r = check_brace(++i);
                        if (r < 0) {
                                return -1;
                        }
                        i = r;
                } else if (is_bool(buff[i])) {

                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        ++i;
                        s_log("check_buff: bool!\n");
                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_op(buff[i]) || is_define(buff[i]) || is_note(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else if (is_alphabet(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        r = check_name(i);
                        if (r < 0) {
                                return -1;
                        }
                        if (is_lbrace(buff[r])) {
                                s_log("check_buff: function begin!\n");
                                p = is_in_ft(i, r);
                                if (p < 0) {
                                	wprintf(L"逻辑联结词未定义!\n");
                                        fputws(L"逻辑联结词未定义!\r\n",fp_output);
                                        output_error(i);
                                        return -1;
                                }
                                r = check_parameter(++r, p);
                                if(r < 0) {
                                        return -1;
                                }
                        } else {
                                s_log("check_buff: var!\n");
                                add_v(i, r);
                        }
                        i = r;
                        while (is_blank(buff[i]))
                                        ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_op(buff[i]) || is_define(buff[i]) || is_note(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else {
                        output_error(i);
                        return -1;
                }
        }
        return 0;
}

//检查括号内的语法
int check_brace(int i)
{
        int r = i;
        int flag = 0;
        int p = 0;
        while (i < length && !is_rbrace(buff[i])) {
                while (i < length && is_blank(buff[i]))
                        ++i;
                if (buff[i] == s_not) {
                        ++i;
                        s_log("check_brace: not!\n");


                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_alphabet(buff[i]) || is_bool(buff[i]) || is_lbrace(buff[i]) || is_not(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }
                } else if (is_op(buff[i])) {
                        if (!flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 0;
                        ++i;
                        s_log("check_brace: operational character!\n");

                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_alphabet(buff[i]) || is_bool(buff[i]) || is_lbrace(buff[i]) || is_not(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else if (is_lbrace(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;

                        r = check_brace(++i);
                        if (r < 0) {
                                return -1;
                        }
                        i = r;

                } else if (is_bool(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        s_log("check_brace: value!\n");
                        ++i;

                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                return 0;
                        }
                        if(is_op(buff[i]) || is_rbrace(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }
                } else if (is_alphabet(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        r = check_name(i);
                        if (r < 0) {
                                return -1;
                        }

                        if (is_lbrace(buff[r])) {
                                p = is_in_ft(i, r);
                                if (p < 0) {
                                        wprintf(L"逻辑联结词未定义!\n");
                                        fputws(L"逻辑联结词未定义!\r\n",fp_output);
                                        output_error(i);
                                        return -1;
                                }
                                r = check_parameter(++r, p);
                                if(r < 0) {
                                        return -1;
                                }
                                s_log("check_brace: function begin!\n");
                        } else {
                                s_log("check_brace: var!\n");
                                add_v(i, r);
                        }
                        i = r;
                        while (is_blank(buff[i]))
                                ++i;

                        if (i >= length) {
                                return 0;
                        }
                        if(is_op(buff[i])  || is_rbrace(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }
                }
        }
        if (i < length && is_rbrace(buff[i])) {
                s_log("check_brace: right brace!\n");
                return ++i;
        }
        output_error(i);
        return -1;
}

//检查函数的参数是否合法
int check_parameter(int i, int index)
{
        int r = i;
        int flag = 0;
        int p = 0;
        int n = 0;

        s_log("begin : check_parameter = %d\n", index);
        while (i < length && !is_rbrace(buff[i])) {
                while (i < length && is_blank(buff[i]))
                        ++i;
                if (is_lbrace(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        ++i;
                        s_log("check_parameter: left brace!\n");
                        r = check_brace(i);
                        if (r < 0) {
                                return r;
                        }
                        i = r;
                        while (i < length && is_blank(buff[i]))
                                ++i;
                        if (is_comma(buff[i])) {
                                ++i;
                                ++n;
                                flag = 0;
                                continue;
                        }
                        if(is_op(buff[i]) || is_rbrace(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }


                } else if (is_bool(buff[i])) {
                        ++i;
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        s_log("check_parameter: bool!\n");
                        while (i < length && is_blank(buff[i]))
                                ++i;
                        if (is_comma(buff[i])) {
                                ++i;
                                ++n;
                                flag = 0;
                                continue;
                        }
                        if(is_op(buff[i]) || is_rbrace(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else if(is_alphabet(buff[i])) {
                        if (flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 1;
                        r = check_name(i);
                        if (r < 0) {
                                return -1;
                        }
                        if (is_lbrace(buff[r])) {
                                s_log("check_parameter: function begin!\n");
                                p = is_in_ft(i, r);
                                if (p < 0) {
                                        wprintf(L"逻辑联结词未定义!\n");
                                        fputws(L"逻辑联结词未定义!\r\n",fp_output);
                                        output_error(i);
                                        return -1;
                                }
                                s_log("fun name : %ls\n", f_table[p].name);
                                r = check_parameter(++r, p);
                                if (r < 0) {
                                        return r;
                                }

                        } else {
                                s_log("check_parameter: var!\n");
                                add_v(i, r);
                        }
                        i = r;
                        while (i < length && is_blank(buff[i]))
                                ++i;
                        if (is_comma(buff[i])) {
                                ++i;
                                ++n;
                                flag = 0;
                                continue;
                        }
                        if(is_op(buff[i]) || is_rbrace(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else if(is_not(buff[i])) {
                        ++i;
                        s_log("check_brace: operational character!\n");

                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                continue;
                        }
                        if(is_alphabet(buff[i]) || is_bool(buff[i]) || is_lbrace(buff[i]) || is_not(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else if(is_op(buff[i])) {
                        if (!flag) {
                                output_error(i);
                                return -1;
                        }
                        flag = 0;
                        ++i;
                        s_log("check_brace: operational character!\n");

                        while (is_blank(buff[i]))
                                ++i;
                        if (i >= length) {
                                continue;
                        }
                        if(is_alphabet(buff[i]) || is_bool(buff[i]) || is_lbrace(buff[i]) || is_not(buff[i])) {
                                continue;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else {
                        output_error(i);
                        return -1;
                }
        }

        if (i < length && is_rbrace(buff[i])) {
                s_log("check_brace: function end!");
                 s_log(" check_parameter = %d\n", index);
                if (n+1 != f_table[index].number && !(f_table[index].number == 0 && n == 0)) {
                        //printf("lack of parameter!\n");
                        wprintf(L"缺少变元!\n");
                        fputws(L"缺少变元!\r\n",fp_output);
                        //printf("n = %d, number = %d\n", n, f_table[index].number);
                        output_error(i-1);
                        return -1;
                }

                return ++i;
        }
        output_error(i);
        return -1;
}

//提取变量或函数名
int check_name(int i)
{
        while((is_alphabet(buff[i])) || (is_number(buff[i]))) {
                ++i;
        }
        return i;
}



int check_number(int i)
{
        while (i < length && is_number(buff[i]))
                ++i;
        return i;
}

//将合法的长数字字符串转换为响应的整数
int wchar_to_int(wchar_t *ws, int len)
{
        int i = 0;
        int sum = 0;
        while (i < len) {
                sum = sum*10 + (ws[i++]-s_0);
        }
        return sum;
}
//检查函数的定义
int check_define()
{
        int r = 0;
        int n = 0;
        int i = 0;
        s_log("check_define:\n");
        while (i<length && !is_define(buff[i])){
                if (is_note(buff[i])) {
                        return length;
                }
        	++i;
        }
        ++i;
        while (i < length) {
                while (i < length && is_blank(buff[i]))
                        ++i;
                if (i >= length) {
                        return i;
                }
                if (is_alphabet(buff[i])) {
                        r = check_name(i);
                        if (r < 0) {
                                output_error(i);
                                return -1;
                        }
                        add_f(i, r);
                        i = r;
                        while (i < length && is_blank(buff[i]))
                                ++i;
                        if (is_number(buff[i])) {
                                r = check_number(i);
                                if (r < 0) {
                                        output_error(i);
                                        return -1;
                                }

                                n = wchar_to_int(buff+i, r-i);
                                if (n > 10) {
                                        output_error(i);
                                        return -1;
                                }
                                f_table[nf-1].number = n;
                                i = r;
                        } else {
                                output_error(i);
                                return -1;
                        }

                        while (i < length && is_blank(buff[i]))
                                ++i;

                        if (is_number(buff[i])) {
                                r = check_number(i);
                                if ((r-i) != pow(2, n)) {
                                        output_error(i);
                                        return -1;
                                }
                                copy_name(f_table[nf-1].result, i, r);
                                //printf("name = %ls\n", f_table[nf-1].result);
                                i = r;
                        } else {
                                output_error(i);
                                return -1;
                        }

                } else if (is_note(buff[i])) {
                        i = length;
                } else {
                        output_error(i);
                        return -1;
                }
        }
        return i;
}

//将当前检查到的变量添加到变量列表中
void add_v(int b, int e)
{
        if (is_in_vt(b, e) < 0) {
                if (nv < MAX) {
                        copy_name(v_table[nv].name, b, e);
                        ++nv;
                }
        }
}

//将当前检查到的函数添加到函数列表中
void add_f(int b, int e)
{
        if (is_in_ft(b, e) < 0) {
                if (nf < MAX) {
                        copy_name(f_table[nf].name, b, e);
                        s_log("fun name: %ls\n", f_table[nf].name);
                        ++nf;
                }
        }
}

void copy_name(wchar_t *name, int b, int e)
{
        int i = 0;
        while (b+i < e) {
                name[i] = buff[b+i];
                ++i;
        }
        name[i] = 0;
}

bool is_same_name(wchar_t *name, int b, int e)
{
        int i = 0;
        int len = wcslen(name);
        s_log("name len = %d\n", len);
        if (len != e-b) {
                return 0;
        }
        while (i < len && b+i < e) {
                if (buff[b+i] == name[i]) {
                        ++i;
                        continue;
                } else {
                        return 0;
                }
        }
        return 1;
}

//判断当前检测到的变量是否存在于变量列表中
int is_in_vt(int b, int e)
{
        int i = 0;
        while (i < nv) {
                if (is_same_name(v_table[i].name, b, e))
                        return i;
                ++i;
        }
        return -1;
}
//判断当前检测到的函数是否存在于变量列表中
int is_in_ft(int b, int e)
{
        int i = 0;
        while (i < nf) {
                if (is_same_name(f_table[i].name, b, e))
                        return i;
                ++i;
        }
        return -1;
}

//计算全局缓冲区buff数组中的表达式
int calculate_buff()
{
        int n = pow(2, nv);
        int i;
        int j;
        int r;
        int t;
        bool v;
        wchar_t wt;
        //s_log("len = %d\n", length);
        s_log("calculate begin: %d\n", n);
        //system("pause");
        for (j = 0; j < n; ++j) {
                i = 0;
                p_op = 0;
                p_value = 0;
                while (i < length) {
                        while (i < length && (is_blank(buff[i]) || is_comma(buff[i])))
                                ++i;
                        if (i >= length) {
                                break;
                        }
                        if (is_lbrace(buff[i])) {
                                s_log("brace: %d %lc\n",i, buff[i]);
                                push_op(buff[i++]);
                        } else if (is_rbrace(buff[i])) {
                                calculate(flag_rb);
                                ++i;
                        } else if (is_alphabet(buff[i])) {
                                s_log("name: %d %lc\n",i, buff[i]);
                                r = check_name(i);
                                if (is_lbrace(buff[r])) {
                                        t = is_in_ft(i, r);
                                        if (t<0) {
                                                wprintf(L"逻辑联结词未定义!\n");
                                                fputws(L"逻辑联结词未定义!\r\n",fp_output);
                                                return -1;
                                        }
                                        s_log("fun: %d \n",t);
                                        calculate(flag_fun);
                                        push_op(s_0 + t);
                                        //push_op(buff[r]);
                                        ++r;
                                } else {
                                        t = is_in_vt(i, r);
                                        s_log("var: %d \n",t);
                                        if (t < 0) {
                                                return -1;
                                        }
                                        push_value(b_table[t][j]);
                                }
                                i = r;
                        } else if (is_bool(buff[i])) {
                                push_value(buff[i++]-s_0);
                        } else if (is_not(buff[i])) {
                                calculate(flag_not);
                                push_op(buff[i]);
                                ++i;
                        } else if (is_and_or(buff[i])) {
                                calculate(flag_and_or);
                                push_op(buff[i]);
                                ++i;
                        } else if (is_xor_eql_imp(buff[i])) {
                                calculate(flag_xor_imp_eql);
                                push_op(buff[i]);
                                ++i;
                        } else if(is_define(buff[i]) || is_note(buff[i])) {
                                break;
                        } else {
                                output_error(i);
                                //wprintf(L"Error : i = %d, %d\n",i, buff[i]);
                                return -1;
                        }
                }
                calculate_end();
                b_table[nv][j] = stack_value[0];
        }
}
int calculate(int flag)
{
        wchar_t wt;
        int r = 0;
        bool v;
        if (p_op > 0) {
                wt = stack_op[p_op-1];
        } else {
                return 0;
        }
        switch (flag) {
        case flag_fun:
                break;
        case flag_not:
                break;
        case flag_lb:
                //calculate(flag_lb);
                break;
        case flag_and_or:
                if (is_not(wt)) {
                        push_value(f_not(pop_value()));
                        pop_op();
                } else if (is_and_or(wt)) {
                        if (wt == s_and) {
                                push_value(f_and(pop_value(), pop_value()));
                        } else {
                                push_value(f_or(pop_value(), pop_value()));
                        }
                        pop_op();
                }

                break;
        case flag_xor_imp_eql:
                if (is_not(wt)) {
                        push_value(f_not(pop_value()));
                        pop_op();
                } else if (is_and_or(wt)) {
                        if (wt == s_and) {
                                push_value(f_and(pop_value(), pop_value()));
                        } else {
                                push_value(f_or(pop_value(), pop_value()));
                        }
                        pop_op();
                } else if (is_xor_eql_imp(wt)) {
                        if (wt == s_xor) {
                                push_value(f_xor(pop_value(), pop_value()));
                        } else if (wt == s_imp) {
                                push_value(f_imp(pop_value(), pop_value()));
                        } else {
                                push_value(f_eql(pop_value(), pop_value()));
                        }
                        pop_op();
                }
                break;
        case flag_rb:
                if (is_not(wt)) {
                        push_value(f_not(pop_value()));
                        pop_op();
                } else if (is_and_or(wt)) {
                        if (wt == s_and) {
                                push_value(f_and(pop_value(), pop_value()));
                        } else {
                                push_value(f_or(pop_value(), pop_value()));
                        }
                        pop_op();
                } else if (is_xor_eql_imp(wt)) {
                        if (wt == s_xor) {
                                push_value(f_xor(pop_value(), pop_value()));
                        } else if (wt == s_imp) {
                                push_value(f_imp(pop_value(), pop_value()));
                        } else {
                                push_value(f_eql(pop_value(), pop_value()));
                        }
                        pop_op();
                } else if (is_number(wt)) {
                        r = calculate_fun(wt);
                        pop_op();
                        return r;
                } else if (is_lbrace(wt)) {
                        pop_op();
                        return r;
                }
                break;
        default:
                return -1;
        }
        if (flag == flag_rb)
                r = calculate(flag);
        return r;
}
int calculate_end()
{
        wchar_t wt;
        s_log("calculate_end: %d, %d\n", p_op, p_value);
        while (p_op > 0) {
                wt = stack_op[p_op-1];
                if (is_not(wt)) {
                        push_value(f_not(pop_value()));
                        pop_op();
                } else if (is_and_or(wt)) {
                        if (wt == s_and) {
                                push_value(f_and(pop_value(), pop_value()));
                        } else {
                                push_value(f_or(pop_value(), pop_value()));
                        }
                        pop_op();
                } else if (is_xor_eql_imp(wt)) {
                        if (wt == s_xor) {
                                push_value(f_xor(pop_value(), pop_value()));
                        } else if (wt == s_imp) {
                                push_value(f_imp(pop_value(), pop_value()));
                        } else {
                                push_value(f_eql(pop_value(), pop_value()));
                        }
                        pop_op();
                } else if (is_number(wt)) {
                        calculate_fun(wt);
                        pop_op();
                }else {
                        s_log("end : %lc\n", wt);
                        system("pause");
                }
        }
        return 0;
}
int calculate_fun(wchar_t wt)
{
        int index = wt - s_0;
        int sum = 0;
        int p = 1;
        int n = f_table[index].number;
        while (n > 0) {
                sum = sum + pop_value() * p;
                p = p*2;
                --n;
        }
        push_value(f_table[index].result[sum] - s_0);
        return 0;
}

//操作符栈和操作数栈的相关操作
void push_op(wchar_t wt)
{
        stack_op[p_op] = wt;
        ++p_op;
        s_log("push_op: %lc, p_op = %d\n", stack_op[p_op-1], p_op);
}
void push_value(bool b)
{
        stack_value[p_value] = b;
        ++p_value;
        s_log("push_value: %d, p_value = %d\n", stack_value[p_value-1], p_value);
}
wchar_t pop_op()
{
        if (p_op > 0) {
                s_log("pop_op: %lc, p_op = %d\n", stack_op[p_op-1], p_op);
                --p_op;
                return stack_op[p_op];
        } else {
                return -1;
        }
}
bool pop_value()
{
        if (p_value > 0) {
                s_log("pop_value: %d, p_value = %d\n", stack_value[p_value-1], p_value);
                --p_value;
                return stack_value[p_value];
        } else {
                return -1;
        }
}

void output_var()
{
        int i = 0;
        printf("var list begin: \n");
        while (i < nv) {
                printf("%d : %ls \n", i, v_table[i].name);
                ++i;
        }
        printf("var list end \n\n");
}

void output_fun()
{
        int i = 0;
        printf("function list begin:\n");
        while (i < nf) {
                printf("%d : %ls %d %ls\n", i, f_table[i].name, f_table[i].number, f_table[i].result);
                ++i;
        }
        printf("function list end\n\n");
}

void output_true_false()
{
        int i = 0;
        int j = 0;
        int n = pow(2, nv);
        int flag_true = 1;
        int flag_false = 1;
        //wprintf(L"\n真或假: \n");
        while(i < n && (flag_true || flag_false)) {
                if (b_table[nv][i] == 0) {
                        flag_true = 0;
                } else {
                        flag_false = 0;
                }
                ++i;
        }

        if (flag_true) {
                wprintf(L"永真!\n");
                fputws(L"永真!\r\n",fp_output);
                return;
        }

        if (flag_false) {
                wprintf(L"永假!\n");
                fputws(L"永假!\r\n",fp_output);
                return;
        }

        wprintf(L"表达式为真时：\n");
        fwprintf(fp_output, L"表达式为真时：\r\n");
        for (j = 0; j < nv; ++j) {
                wprintf(L"%ls ", v_table[j].name);
                fwprintf(fp_output, L"%ls ", v_table[j].name);
        }
        wprintf(L"\n");
        fwprintf(fp_output, L"\r\n");
        for (i = 0; i < n; ++i) {
                if (b_table[nv][i]) {
                        for (j = 0; j < nv; ++j) {
                                wprintf(L"%d ", b_table[j][i]);
                                fwprintf(fp_output, L"%d ", b_table[j][i]);
                        }
                        wprintf(L"\n");
                        fputws(L"\r\n",fp_output);
                }
        }

        wprintf(L"\n");
        fputws(L"\r\n",fp_output);
}

void output_value()
{
        int i;
        int n = pow(2, nv);
        printf("\nvalue list: \n");
        for (i = 0; i < n; ++i) {
                printf("%d ", b_table[nv][i]);
        }
        printf("\n");
}

void init_table()
{
        int i, j, k;
        bool t;
        int d;
        int n = pow(2, nv);
        for(i = 0; i < nv; ++i){
                k = 0;
                t = 0;
                d = n/(pow(2, i+1));
                for(j = 0; j < n; ++j){
                        if (k == d) {
                                t = !t;
                                k = 0;
                        }
                        ++k;
                        b_table[i][j] = t;
                }
        }

        for(j = 0; j < n; ++j){
                b_table[i][j] = 0;
        }

}

void output_table()
{
        int i, j;
        int n = pow(2, nv);
        for(i = 0; i < nv; ++i){
                printf("%ls :", v_table[i]);
                for(j = 0; j < n; ++j){
                        printf("%d ", b_table[i][j]);
                }
                printf("\n");
        }
}

void output_error(int i)
{
        int j = 0;
        wprintf(L"Error: \n");
        fwprintf(fp_output, L"Error :\r\n");
        while(j < i) {
                wprintf(L"%lc", buff[j]);
                fwprintf(fp_output, L"%lc", buff[j]);
                ++j;
        }
        wprintf(L"?");
        fwprintf(fp_output, L"?");
        while (j < length) {
                wprintf(L"%lc", buff[j]);
                fwprintf(fp_output, L"%lc", buff[j]);
                ++j;
        }
        //printf("i:%d, %d\n", i, buff[i]);
}



int copy_str(wchar_t *l, wchar_t *r, int len)
{
        int i = 0;
        while (i < len) {
                l[i] = r[i];
                ++i;
        }
        l[i] = 0;
        return i;
}

static inline void clear_set16()
{
        begin_set = 0;
        end_set = 2;
        len_set = 2;
}

void init_set16()
{
        clear_set16();
        copy_str(set16[0].str, L"p", 1);
        copy_str(set16[0].result, L"0011", 4);
        copy_str(set16[1].str, L"q", 1);
        copy_str(set16[1].result, L"0101", 4);
}

void init_vtable()
{
        copy_str(v_table[0].name, L"p", 1);
        copy_str(v_table[1].name, L"q", 1);
        nv = 2;
        init_table();
}


void get_set()
{
        int i,j;
        int s = 0;
        init_set16();
        //output_set();
        init_vtable();
        while(1) {
                //begin_set表示上一轮生成第一个公式所在的位置，
                //end_set表示上一轮生成最后一个公式所在位置的后一个位置
                //len_set当前集合中有多少个不等价公式
                //若上一轮没有生成公式则结束
                if (begin_set == end_set) {
                        break;
                }
                //使用集合中的不等价公式来生成新的不等价公式
                //当已经找到16个公式时，返回1，此时结束搜索
                s = generate_str();
                if (s) {
                        break;
                }
                begin_set = end_set;
                end_set = len_set;
        }

        //输出最终求完全集的结果
        output_set();
}

int generate_str()
{
        int i;
        int s = 0;
        //nf是新定义的联结词个数，使用集合中的不等价公式分别做联结词的操作数来生成新公式。
        for (i = 0; i < nf; ++i) {
                s = swap_var(i);
                if (s) {
                        return 1;
                }
        }
        return 0;
}


int swap_var(int p)
{
        int i,j,k;
        int number = f_table[p].number;
        int index = 0;
        int s = 0;
        int flag = 1;


        int n = pow(end_set, number);
        //printf("n = %d\n", n);
        //依次替换函数的参数，生成新公式
        for (i = 0; i < n; ++i) {
                index = 0;
                flag = 1;
                index = copy_str(buff+index, f_table[p].name, wcslen(f_table[p].name));
                buff[index++] = L'(';
                //printf("i = %d, end_set = %d len_set = %d, number = %d\n", i, end_set,len_set, number);
                for (j = 0; j < number; ++j) {
                        k = (int)(i / (int)pow(end_set, j)) % end_set;
                        //printf(" %d ", k);
                        if (flag && k > begin_set) {
                                flag = 0;
                        }
                        index += copy_str(buff+index, set16[k].str, wcslen(set16[k].str));

                        if (j < number-1) {
                                buff[index++] = L',';
                        }

                }
                buff[index++] = L')';
                buff[index] = 0;
                //printf("\n");
                /*
                if (flag) {
                        continue;
                }
                */
                //将生成的公式添加进集合
                s = add_set();
                if (s) {
                        return s;
                }
        }
        return 0;
}

int add_set()
{
        int i = 0;
        int index = 0;
        wchar_t v[4] = L"0000";
        length = wcslen(buff);

        calculate_buff();
        for (i = 0; i < 4; ++i) {
                v[i] += b_table[nv][i];
        }
        //printf("buff = %ls  ", buff);
        //printf("v = %ls ", v);
        index = is_in_set(v);
        //printf("index = %d\n", index);
        if (index < 0) {
                add_to_set(len_set, v);
                ++len_set;
        }
        if (len_set == 16) {
                return 1;
        }
        return 0;
}

int is_in_set(wchar_t *t)
{
        int i = 0;
        int s = 0;
        for ( ; i < len_set; ++i) {
                s = is_same_v(set16[i].result, t, 4);
                if (s) {
                        return i;
                }
        }
        return -1;
}

void add_to_set(int len, wchar_t *t)
{
        copy_str(set16[len].str, buff, wcslen(buff));
        copy_str(set16[len].result, t, 4);
}


bool is_same_v(wchar_t *l, wchar_t *r, int len)
{
        int i = 0;
        while (i < len) {
                if (l[i] == r[i]) {
                        ++i;
                        continue;
                } else {
                        return 0;
                }
        }
        return 1;
}

void output_set()
{
        int i = 0;
        if (len_set == 16) {
                wprintf(L"完全集\n");
                fputws(L"完全集\r\n",fp_output);
        } else {
                wprintf(L"非完全集\n");
                fputws(L"非完全集\r\n",fp_output);
        }
        //printf("begin = %d, end = %d, len = %d\n", begin_set, end_set, len_set);
        while (i < len_set) {
                //wprintf(L"%ls ：%ls", set16[i].str, set16[i].result);
                wprintf(L"%d: %ls \n", i + 1, set16[i].str);
                fwprintf(fp_output, L"%d: %ls \r\n", i + 1, set16[i].str);
                ++i;
        }
        wprintf(L"\n");
        fputws(L"\r\n",fp_output);
        if (len_set == 16) {
                output_not_imp();
        }
}
void output_not_imp()
{
        int i = 0;
        wprintf(L"\n");
        fputws(L"\r\n",fp_output);
        while (i < len_set) {
                if (is_same_v(set16[i].result, L"1100", 4)) {
                        wprintf(L"¬p ↔ %ls\n", set16[i].str);
                        fwprintf(fp_output, L"¬p ↔ %ls\r\n", set16[i].str);
                }
                if (is_same_v(set16[i].result, L"1101", 4)) {
                        wprintf(L"p→q ↔  %ls\n", set16[i].str);
                        fwprintf(fp_output, L"p→q ↔  %ls\r\n", set16[i].str);
                }
                ++i;
        }
}
