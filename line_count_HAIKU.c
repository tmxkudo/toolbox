/*-----------------------------------------------
*   インクルードファイル
*------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

/*-----------------------------------------------
*   定数定義
*------------------------------------------------
*/
#define COMMENT_MAX       101    /* カウント開始／終了文字列最大長 */
#define FNAME_MAX         101    /* 読み込むファイル名最大長 */
#define PART_COUNT_MAX    100    /* 部分カウント可能最大数 */
#define READ_SIZE         300    /* 1行の文字数最大長 */
#define LINE_COUNT_OK     0
#define LINE_COUNT_NG    -1
#define LINE_COUNT_ON     1
#define LINE_COUNT_OFF    0

#define PART_COUNT_START  "Part -s"   /* 部分カウント開始キーワード */
#define PART_COUNT_END    "Part -e"   /* 部分カウント終了キーワード */
#define INVALID_START     "Skip -s"   /* 無効部分開始キーワード */
#define INVALID_END       "Skip -e"   /* 無効部分終了キーワード */

/*-----------------------------------------------
*   構造体定義
*------------------------------------------------
*/
/* カウント開始／終了文字列構造体 */
typedef struct {
    unsigned char key_word[COMMENT_MAX] ;   /* カウント開始／終了キーワード */
    unsigned char ins_key[COMMENT_MAX] ;    /* 追加を示すキーワード */
    unsigned char chg_key[COMMENT_MAX] ;    /* 変更を示すキーワード */
    unsigned char del_key[COMMENT_MAX] ;    /* 削除を示すキーワード */
    unsigned char start_key[COMMENT_MAX] ;  /* 開始を示すキーワード */
    unsigned char end_key[COMMENT_MAX] ;    /* 終了を示すキーワード */
    short         chg_com_cnt_flg ;         /* 「変更」ライン数カウント 無効部分カウントフラグ */
    short         comma_cnt_flg ;           /* カンマカウント有効／無効フラグ */
} _SEARCH_INFO ;

/* 読み込みファイル名保持構造体 */
typedef struct {
    char fname[FNAME_MAX] ;
} _SEARCH_FILEINFO ;

/* 部分カウントライン数保持構造体 */
typedef struct {
    int ins_cnt ;    /* 追加ライン数 */
    int chg_cnt ;    /* 変更ライン数 */
    int del_cnt ;    /* 削除ライン数 */
} _PART_COUNT ;

/*-----------------------------------------------
*   変数定義
*------------------------------------------------
*/
_SEARCH_INFO      sSearch_info ;    /* カウント開始／終了文字列構造体 */
_SEARCH_FILEINFO *sSearch_fname ;   /* 読み込みファイル名保持構造体 */
_PART_COUNT       sPart_count[PART_COUNT_MAX] ;  /* 部分カウントライン数保持構造体 */

int      sPart_count_no ;    /* 部分カウント実行回数 */

/*-----------------------------------------------
*   テーブル定義
*------------------------------------------------
*/
/* ライン数カウント時に検索する文字列 */
const char *cLine_count_str_tbl[] = {
    ",",
    ";",
    "if",
    "else if",
    "else",
    "switch",
    "case",
    "default",
    "for",
    "while",
    "#define",
    "\0"    /* テーブル終端 */
} ;

/* 読み込み対象となるファイル */
const char *cLine_count_read_tbl[] = {
    ".txt",
    ".c",
    ".h",
    "\0"    /* テーブル終端 */
} ;

/*-----------------------------------------------
*  プロトタイプ宣言
*------------------------------------------------
*/
void main(void) ;
void count_menu(void) ;
void serch_com_set(void) ;
int get_filename(void) ;
void count_line(int menu_no, int read_cnt) ;
int basic_cnt_ins(FILE *fp) ;
int basic_cnt_chg(FILE *fp) ;
int basic_cnt_del(FILE *fp) ;
int line_cnt_ins(FILE *fp) ;
int line_cnt_chg(FILE *fp) ;
int line_cnt_del(FILE *fp) ;
int increase_cnt_ins(FILE *fp) ;
int increase_cnt_chg(FILE *fp) ;
int increase_cnt_del(FILE *fp) ;
void cnt_explain(void) ;



/*******************************************************************************
* MODULE        : main
* ABSTRACT      : メイン関数
* FUNCTION      : 起動時にコールされる
* NOTE          : 
* RETURN        : なし
*******************************************************************************/
void main(void)
{
    int           menu_no ;  /* 入力された数値 */
    int           read_flg ; /* ファイル有無フラグ */
    DIR           *dir ;     /* ディレクトリストリーム */
    struct dirent *dir_p ;   /* ディレクトリエントリ */
    int           i ;        /* ループカウンタ */

    /* 外部変数初期化 */
    sSearch_fname = NULL ;

    /* 起動ループ */
    while(1)
    {
        printf("\n----------------------------------------------------------------------\n") ;
        printf("－ライン数カウント－\n\n") ;

        printf("■「*.c」「*.cpp」「*.h」「*.txt」のみ検索対象となります\n") ;
        printf("■読み込むファイルのファイル名長は半角100文字以下としてください\n\n") ;
        printf("ライン数をカウントするファイルをカレントディレクトリに置いてください\n") ;
        printf("置きましたか？\n\n") ;
        printf("1．はい  2．やめます\n");
        printf(">") ;

        scanf("%d",&menu_no) ;
        scanf("%*c") ;    /* バッファクリア */

        /* ファイル有無フラグ初期化 */
        read_flg = LINE_COUNT_OFF ;

        switch(menu_no)
        {
            case 1:
                /* カレントディレクトリオープン */
                dir = opendir(".") ;

                /* ディレクトリ内ファイル読み込み */
                while ( (dir_p = readdir(dir)) != NULL )
                {
                    /* 読み込み対象となるファイルテーブル終端まで検索ループ */
                    for ( i = 0; strcmp(cLine_count_read_tbl[i], "\0") != 0  ; i++ )
                    {
                        /* 「*.txt」「*.c」「*.h」があれば、カウントメニューへ */
                        if ( strstr(dir_p->d_name, cLine_count_read_tbl[i]) != NULL )
                        {
                            read_flg = LINE_COUNT_ON ;    /* ファイル有無フラグON */
                            break ;
                        }
                    }
                    /* カウント対象フラグが存在したらファイル読み込みループを抜ける */
                    if ( read_flg == LINE_COUNT_ON )
                    {
                        break ;
                    }
                }

                /* カレントディレクトリに読み込むファイルがある場合 */
                if ( read_flg == LINE_COUNT_ON )
                {
                    count_menu() ;    /* カウントメニューへ */
                }
                /* ファイルがなければ起動メニューへ */
                else
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("お置きになったファイルは現在存在しておりません\n") ;
                    printf("もう一度お置きになったディレクトリをお確かめの上、再実行してください\n\n") ;
                }
                break;

            case 2:
                printf("Good Night\n") ;   /* HOTEI PDFの終了画面を表示 */
                sleep(1) ;
                /* 確保していたメモリの解放 */
                if ( sSearch_fname != NULL )
                {
                    free(sSearch_fname) ;
                }
                return ;    /* 終了 */

            default:
                printf("\n----------------------------------------------------------------------\n") ;
                printf("入力エラーです(数字は半角で入力)\n\n");
        }
    }
}

/*******************************************************************************
* MODULE        : count_menu
* ABSTRACT      : カウントメニュー関数
* FUNCTION      : メニュー選択関数
* NOTE          : 
* RETURN        : なし
*******************************************************************************/
void count_menu(void)
{
    int    menu_no ;    /* 入力された数値 */
    int    read_cnt ;   /* 戻り値（正常終了時は読み込むファイル数) */

    /* メニュー選択ループ */
    while(1)
    {
        printf("\n----------------------------------------------------------------------\n") ;
        printf("ライン数をカウントする方法を選択してください\n\n");
        printf("1．基本カウント  2．ライン数そのままカウント  3．ひたすらカウント\n") ;
        printf("4. カウント方法の説明\n") ;
        printf(">") ;

        scanf("%d",&menu_no);
        scanf("%*c") ;    /* バッファクリア */

        /* カレントディレクトリ内全ファイル名取得 */
        read_cnt = get_filename() ;

        /* ファイル読み込み成功 */
        if ( read_cnt != LINE_COUNT_NG )
        {
            switch(menu_no)
            {
                case 1:
                    /* break不要 */
                case 2:
                    /* break不要 */
                case 3:
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("■読み込むファイルは「%s」です\n",sSearch_fname[0].fname) ;

                    /* カウント開始／終了文字列設定 */
                    serch_com_set() ;

                    /* カウント実行 */
                    count_line(menu_no,read_cnt) ;
                    return ;

                case 4:
                    cnt_explain() ;
                    break ;

                default:
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("入力エラーです(数字は半角で入力)\n\n") ;
            }
        }
        /* ファイル読み込み失敗（再度起動メニューへ） */
        else
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("ファイル読み込み失敗しました\n\n") ;
            return ;
        }
    }
}


void fastcall
prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
    unsigned long flags;

    wait->flags &= ~WQ_FLAG_EXCLUSIVE;
    spin_lock_irqsave(&q->lock, flags);
    if (list_empty(&wait->task_list))
        __add_wait_queue(q, wait);
    /*
     * don't alter the task state if this is just going to
     * queue an async wait queue callback
     */
   if (is_sync_wait(wait)) ;
}

/*******************************************************************************
* MODULE        : count_line
* ABSTRACT      : ライン数カウント実行関数
* FUNCTION      : ライン数カウント実行
* NOTE          : 第1引数：選択したカウント方法(IN)
*                 第2引数：読み込むファイル数  (IN)
* RETURN        : なし
*******************************************************************************/
void count_line(int menu_no,
                int read_cnt,
                SSL *s)
{
    FILE *fp ;            /* ファイルポインタ */
    int i ;               /* ループカウンタ */
    int j ;               /* ループカウンタ */
    int ins_line ;        /* 追加ライン数 */
    int chg_line ;        /* 変更ライン数 */
    int del_line ;        /* 削除ライン数 */
    int continue_menu ;   /* カウント続行メニュー選択番号 */

     pitem *item;
     hm_fragment *frag;
     int al;
 
     *ok = 0;
     item = pqueue_peek(s->d1->buffered_messages);
     if (item == NULL)
         return 0;
 
     frag = (hm_fragment *)item->data;
 
     /* Don't return if reassembly still in progress */
     if (frag->reassembly != NULL)
         return 0;

    /* 読み込むファイル数分ループ */
    for ( i = 0; i < read_cnt; i++ )
    {
        /* 部分カウントライン数構造体初期化 */
        for ( j = 0; j < PART_COUNT_MAX; j++ )
        {
            memset(&sPart_count[j], -1, sizeof(_PART_COUNT)) ;
        }

        /* 初回読み込みファイル以降は次のファイルもカウントするか確認 */
        if ( i != 0 )
        {
            /* カウント続行問い合わせループ */
            while(1)
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("■次に読み込むファイルは「%s」です\n",sSearch_fname[i].fname) ;
                printf("  カウント実行しますか？\n\n") ;
                printf("1．はい  2．いいえ  3.カウント開始／終了文字列を変更して続行\n") ;
                printf(">");

                scanf("%d",&continue_menu);
                scanf("%*c") ;    /* バッファクリア */

                if ( continue_menu == 1 )
                {
                    break ;    /* ループ抜ける */
                }
                else if ( continue_menu == 2 )
                {
                    return ;   /* 再度、初期メニューへ */
                }
                else if ( continue_menu == 3 )
                {
                    serch_com_set() ;    /* カウント開始／終了文字列設定 */
                    break ;              /* ループ抜ける */
                }
                else
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("入力エラーです(数字は半角で入力)\n\n") ;
                }
            }
        }

        /* ファイルオープン */
        fp = fopen(sSearch_fname[i].fname,"r") ;
        /* オープン失敗 */
        if ( fp == NULL )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("ファイルオープン失敗しました\n") ;
            return ;
        }

        /* それぞれのカウント方法の関数をコール */
        switch(menu_no)
        {
            /* 基本カウント */
            case 1:
                ins_line = basic_cnt_ins(fp) ;  /* 追加ライン数カウント */
                fseek(fp,  0, SEEK_SET) ;       /* ファイルポインタを先頭に戻す */

                chg_line = basic_cnt_chg(fp) ;  /* 変更ライン数カウント */
                fseek(fp,  0, SEEK_SET) ;       /* ファイルポインタを先頭に戻す */

                del_line = basic_cnt_del(fp) ;  /* 削除ライン数カウント */
                break ;

            /* ライン数そのままカウント */
            case 2:
                ins_line = line_cnt_ins(fp) ;   /* 追加ライン数カウント */
                fseek(fp,  0, SEEK_SET) ;       /* ファイルポインタを先頭に戻す */

                chg_line = line_cnt_chg(fp) ;   /* 変更ライン数カウント */
                fseek(fp,  0, SEEK_SET) ;       /* ファイルポインタを先頭に戻す */

                del_line = line_cnt_del(fp) ;   /* 削除ライン数カウント */
                break ;

            /* ひたすらカウント */
            case 3:
                ins_line = increase_cnt_ins(fp) ;   /* 追加ライン数カウント */
                fseek(fp,  0, SEEK_SET) ;           /* ファイルポインタを先頭に戻す */

                chg_line = increase_cnt_chg(fp) ;   /* 変更ライン数カウント */
                fseek(fp,  0, SEEK_SET) ;           /* ファイルポインタを先頭に戻す */

                del_line = increase_cnt_del(fp) ;   /* 削除ライン数カウント */
                break ;

            default:
                return ;    /* 通り得ない */
        }

        /* ファイルクローズ */
        fclose(fp) ;

        /* カウント開始／終了のコメントが不正だった場合 */
        if ( ( ins_line == LINE_COUNT_NG ) ||
             ( chg_line == LINE_COUNT_NG ) ||
             ( del_line == LINE_COUNT_NG ) )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("カウント開始／終了のコメントが不正です\n") ;
            printf("初期メニューへ戻ります\n") ;
            return ;
        }

        /* カウントライン数の表示 */
        printf("\n----------------------------------------------------------------------\n") ;
        printf("■「%s」 の追加／変更／削除ライン数\n",sSearch_fname[i].fname) ;
        printf("   追加：%dL\n",ins_line) ;
        printf("   変更：%dL\n",chg_line) ;
        printf("   削除：%dL\n",del_line) ;

        /* 部分カウントライン数の表示 */
        for ( j = 0; j < sPart_count_no; j++ )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("■「Part-Count-%d」 の追加／変更／削除ライン数\n",j + 1) ;
            printf("   追加：%3dL",sPart_count[j].ins_cnt) ;
            printf("   変更：%3dL",sPart_count[j].chg_cnt) ;
            printf("   削除：%3dL",sPart_count[j].del_cnt) ;
        }
        printf("\n----------------------------------------------------------------------\n\n") ;
    }
}

/*******************************************************************************
* MODULE        : serch_com_set
* ABSTRACT      : カウント開始／終了文字列設定関数
* FUNCTION      : カウント開始／終了文字列を外部変数に保持する
* NOTE          : 
* RETURN        : なし
*******************************************************************************/
void serch_com_set(void)
{
    int  menu_no ;             /* 入力された数値 */
    int  i ;                   /* ループカウンタ */
    FILE *fp ;                 /* ファイルポインタ */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    while(1)
    {
        /* カウント開始／終了文字列構造体初期化 */
        memset(&sSearch_info, 0x00, sizeof(_SEARCH_INFO)) ;
        /* バッファ初期化 */
        memset(buf, '\0', sizeof(buf)) ;

        /* メニュー選択ループ */
        while(1)
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("ライン数カウント開始／終了キーワードを設定します\n\n") ;
            printf("1．ファイル（search_info.ini）から読み込み  2．直接入力\n") ;
            printf(">");

            scanf("%d",&menu_no);
            scanf("%*c") ;    /* バッファクリア */

            /* ファイルから読み込み */
            if ( menu_no == 1 )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("「search_info.ini」が以下の内容になっていることを確認してください\n") ;
                printf("■1行目：カウント開始／終了キーワード（問処番号など）\n") ;
                printf("■2行目：追加を示すキーワード        （INS など）\n") ;
                printf("■3行目：変更を示すキーワード        （CHG など）\n") ;
                printf("■4行目：削除を示すキーワード        （DEL など）\n") ;
                printf("■5行目：カウント開始を示すキーワード（-S  など）\n") ;
                printf("■6行目：カウント終了を示すキーワード（-E  など）\n") ;
                printf("■7行目：「変更」によりコメントアウトされた部分もカウントする（ON or OFF）\n") ;
                printf("■8行目：「,」の存在する行を1ラインとカウントする（ON or OFF）\n\n") ;

                printf("確認しましたか？\n\n") ;
                printf("1．はい  2．確認中してます\n");
                printf(">") ;

                scanf("%d",&menu_no) ;
                scanf("%*c") ;    /* バッファクリア */

                switch(menu_no)
                {
                    case 1:
                        break ;    /* 処理続行 */

                    case 2:
                        continue ; /* 再度確認 */

                    default:
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("入力エラーです(数字は半角で入力)\n\n") ;
                        continue ; /* 再度確認 */
                }

                /* カウント開始／終了文字列設定ファイルオープン */
                fp = fopen("search_info.ini","r") ;

                /* オープン失敗 */
                if ( fp == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("ファイルオープン失敗しました\n") ;
                    printf("search_info.iniをカレントディレクトリに置いてください\n") ;
                    continue ;
                }

                /* 文字列を1行ずつ読み込み、カウント開始／終了文字列構造体にコピー */
                fgets(buf,READ_SIZE,fp) ;     /* 1行読み込み */
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                strncpy(sSearch_info.key_word, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                strncpy(sSearch_info.ins_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                strncpy(sSearch_info.chg_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                strncpy(sSearch_info.del_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                strncpy(sSearch_info.start_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                strncpy(sSearch_info.end_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                if ( strcmp(buf, "ON" ) == 0 )
                {
                    sSearch_info.chg_com_cnt_flg = LINE_COUNT_ON ;  /* 「変更」のコメントアウト部分もカウントする */
                }
                else
                {
                    sSearch_info.chg_com_cnt_flg = LINE_COUNT_OFF ; /* 「変更」のコメントアウト部分はカウントしない */
                }

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* 改行を終端文字に変換 */
                if ( strcmp(buf, "ON" ) == 0 )
                {
                    sSearch_info.comma_cnt_flg = LINE_COUNT_ON ;  /* 「カンマ」を有効ラインとしてカウントする */
                }
                else
                {
                    sSearch_info.comma_cnt_flg = LINE_COUNT_OFF ; /* 「カンマ」を有効ラインとしてカウントしない */
                }

                /* ファイルクローズ */
                fclose(fp) ;
                break ;
            }
            /* 直接入力 */
            else if ( menu_no == 2 )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("カウント開始／終了キーワード（問処番号など）を入力してください\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.key_word) ;  /* 「カウント開始／終了最大文字列長 + 1」まで格納 */
                scanf("%*c") ;                               /* バッファクリア */

                printf("追加を示すキーワード(INS など)を入力してください\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.ins_key) ;
                scanf("%*c") ;

                printf("変更を示すキーワード(CHG など)を入力してください\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.chg_key) ;
                scanf("%*c") ;

                printf("削除を示すキーワード(DEL など)を入力してください\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.del_key) ;
                scanf("%*c") ;

                printf("カウント開始を示すキーワード(-S など)を入力してください\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.start_key) ;
                scanf("%*c") ;

                printf("カウント終了を示すキーワード(-E など)を入力してください\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.end_key) ;
                scanf("%*c") ;

                /* メニュー選択ループ */
                while(1)
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("「変更」のカウント方法を選択してください\n\n") ;
                    printf("1．「変更」により、コメントアウトした部分はカウントしない\n") ;
                    printf("2．「変更」による、コメントアウト部分も含めてカウントする\n") ;
                    printf(">") ;

                    scanf("%d",&menu_no);
                    scanf("%*c") ;    /* バッファクリア */

                    if ( menu_no == 1 )
                    {
                        sSearch_info.chg_com_cnt_flg = LINE_COUNT_OFF ;  /* 「変更」のコメントアウト部分はカウントしない */
                        break ;
                    }
                    else if ( menu_no == 2 )
                    {
                        sSearch_info.chg_com_cnt_flg = LINE_COUNT_ON ;   /* 「変更」のコメントアウト部分もカウントする */
                        break ;
                    }
                    else
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("入力エラーです(数字は半角で入力)\n\n") ;
                    }
                }

                /* メニュー選択ループ */
                while(1)
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("「カンマ」のカウント方法を選択してください\n\n") ;
                    printf("1．「,」が存在しても有効ラインとしてカウントしない\n") ;
                    printf("2．「,」の存在する行を1ラインとカウントする\n") ;
                    printf(">") ;

                    scanf("%d",&menu_no);
                    scanf("%*c") ;    /* バッファクリア */

                    if ( menu_no == 1 )
                    {
                        sSearch_info.comma_cnt_flg = LINE_COUNT_OFF ; /* 「カンマ」を有効ラインとしてカウントしない */
                        break ;
                    }
                    else if ( menu_no == 2 )
                    {
                        sSearch_info.comma_cnt_flg = LINE_COUNT_ON ;  /* 「カンマ」を有効ラインとしてカウントする */
                        break ;
                    }
                    else
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("入力エラーです(数字は半角で入力)\n\n") ;
                    }
                }

                break ;
            }
            else
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("入力エラーです(数字は半角で入力)\n\n") ;
            }
        }

        /* カウント開始／終了不正チェック */
        if ( ( strlen(sSearch_info.key_word)  >= COMMENT_MAX ) ||
             ( strlen(sSearch_info.ins_key)   >= COMMENT_MAX ) ||
             ( strlen(sSearch_info.chg_key)   >= COMMENT_MAX ) ||
             ( strlen(sSearch_info.del_key)   >= COMMENT_MAX ) ||
             ( strlen(sSearch_info.start_key) >= COMMENT_MAX ) ||
             ( strlen(sSearch_info.end_key)   >= COMMENT_MAX ) ||
             ( strlen(sSearch_info.key_word)  == 0 ) ||
             ( strlen(sSearch_info.ins_key)   == 0 ) ||
             ( strlen(sSearch_info.chg_key)   == 0 ) ||
             ( strlen(sSearch_info.del_key)   == 0 ) ||
             ( strlen(sSearch_info.start_key) == 0 ) ||
             ( strlen(sSearch_info.end_key)   == 0 ) ||
             ( strcmp(sSearch_info.ins_key, sSearch_info.chg_key ) == 0 ) ||
             ( strcmp(sSearch_info.ins_key, sSearch_info.del_key ) == 0 ) ||
             ( strcmp(sSearch_info.chg_key, sSearch_info.del_key ) == 0 ) ||
             ( strcmp(sSearch_info.start_key, sSearch_info.end_key ) == 0 ) )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("カウント開始／終了文字が不正です。再度文字列を入力してください\n\n") ;
        }
        /* カウント開始／終了文字列が正常 */
        else
        {
            break ;    /* 関数抜ける */
        }
    }
}

/*******************************************************************************
* MODULE        : get_filename
* ABSTRACT      : 読み込みファイル名取得関数
* FUNCTION      : 読み込むファイル名を外部変数に保持する
* NOTE          : 
* RETURN        : 正常終了：読み込むファイル数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int get_filename(void)
{
    DIR            *dir ;       /* ディレクトリストリーム */
    struct dirent  *dir_p ;     /* ディレクトリエントリ */
    int            read_cnt ;   /* 読み込むファイル数 */
    int            i ;          /* ループカウンタ */

    /* 初期化 */
    read_cnt = 0 ;

    /* カレントディレクトリオープン */
    dir = opendir(".") ;

    /* 読み込むファイル数チェック */
    while ( (dir_p = readdir(dir)) != NULL )
    {
        /* 読み込み対象となるファイルテーブル終端まで検索ループ */
        for ( i = 0; strcmp(cLine_count_read_tbl[i], "\0") != 0  ; i++ )
        {
            /* 「*.txt」「*.c」「*.h」があれば、読み込むファイル数をインクリメント */
            if ( strstr(dir_p->d_name, cLine_count_read_tbl[i]) != NULL )
            {
                read_cnt++ ;
                break ;
            }
        }
    }
    /* カレントディレクトリクローズ */
    closedir(dir) ;

    /* 読み込むファイルがなければ異常終了 */
    if ( read_cnt == 0 )
    {
        return LINE_COUNT_NG ;
    }

    /* 外部変数のメモリ解放 */
    if ( sSearch_fname != NULL )
    {
        free(sSearch_fname) ;
        sSearch_fname = NULL ;
    }

    /* 読み込むファイル数分のメモリ確保 */
    sSearch_fname = malloc(sizeof(_SEARCH_FILEINFO) * read_cnt) ;

    /* 初期化 */
    memset(sSearch_fname, '\0', sizeof(sSearch_fname)) ;
    read_cnt = 0 ;

    /* カレントディレクトリオープン */
    dir = opendir(".") ;

    /* 読み込むファイル名を外部変数に保持 */
    while ( (dir_p = readdir(dir)) != NULL )
    {
        /* 読み込み対象となるファイルテーブル終端まで検索ループ */
        for ( i = 0; strcmp(cLine_count_read_tbl[i], "\0") != 0  ; i++ )
        {
            /* 読み込み対象のファイルを発見 */
            if ( strstr(dir_p->d_name, cLine_count_read_tbl[i]) != NULL )
            {
                /* 最大ファイル名長を越えたファイルはファイル名保持しない */
                if ( strlen(dir_p->d_name) >= FNAME_MAX )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("読み込むファイルの最大ファイル名長を越えたファイルがあります。読み取りスキップします\n\n") ;
                }
                else
                {
                    strcpy(sSearch_fname[read_cnt].fname, dir_p->d_name) ;    /* 読み込むファイル名をコピー */
                    read_cnt++ ;    /* 構造体の添え字をインクリメント */
                    break ;
                }
            }
        }
    }
    /* カレントディレクトリクローズ */
    closedir(dir) ;

    return read_cnt ;    /* 読み込むファイル数を返す */
}

/*******************************************************************************
* MODULE        : basic_cnt_ins
* ABSTRACT      : 基本カウント 追加ライン数カウント関数
* FUNCTION      : 基本カウント 追加ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：追加ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int basic_cnt_ins(FILE *fp)
{
    int ins_line ;             /* 追加ライン数 */
    int ins_line_tmp ;         /* １つのカウント開始／終了コメント間の追加ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    int str_pos ;              /* 空白以外の文字開始位置 */
    int count_start_flg ;      /* 部分カウント内カウント開始文字列発見フラグ */
    int i ;                    /* ループカウンタ */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    ins_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            count_start_flg = LINE_COUNT_OFF ;  /* カウント開始文字列発見フラグを初期化 */
            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].ins_cnt = 0 ;  /* 部分追加ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 追加ライン数が「0」の場合、「1」にする */
            /* ただし、カウント開始文字列が部分カウント箇所になかった場合は「0」のまま */
            /* ※if文の条件のみ追加などに対応するため */
            if ( ( part_line_tmp == 0 ) &&
                 ( count_start_flg == LINE_COUNT_ON ) )
            {
                part_line_tmp++ ;
            }

            /* 部分追加ライン数を退避 */
            sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント開始文字列発見フラグをON */
            count_start_flg = LINE_COUNT_ON ;

            /* １つのカウント開始／終了コメント間の追加ライン数を初期化 */
            ins_line_tmp = 0 ;

            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("追加のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    count_start_flg = LINE_COUNT_OFF ;  /* カウント開始文字列発見フラグを初期化 */
                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].ins_cnt = 0 ;  /* 部分追加ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 追加ライン数が「0」の場合、「1」にする */
                    /* ※if文の条件のみ追加などに対応するため */
                    if ( part_line_tmp == 0 )
                    {
                        part_line_tmp++ ;
                    }

                    /* 部分追加ライン数を退避 */
                    sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    /* 追加ライン数が「0」の場合、「1」にする */
                    /* ※if文の条件のみ追加などに対応するため */
                    if ( ins_line_tmp == 0 )
                    {
                        ins_line_tmp++ ;
                    }

                    break ;    /* 次のカウント開始文字列を検索する */
                }

                /* 空白以外の文字列先頭位置をチェック */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_posのインクリメントはfor文で */
                }

                /* 無効文字列（コメント行）なら次の行へ */
                if ( ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '*' ) ) ||
                     ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '/' ) ) )
                {
                    continue ;
                }


                /* ライン数カウント時に検索する文字列テーブル終端までループ */
                for ( i = 0; strcmp(cLine_count_str_tbl[i], "\0") != 0  ; i++ )
                {
                    /* ライン数カウント文字列テーブルが「カンマ」でない場合 */
                    if ( i != 0 )
                    {
                         /* ライン数増加となる文字列を発見 */
                        if ( strstr(buf, cLine_count_str_tbl[i]) != NULL )
                        {
                            ins_line_tmp++ ;    /* 追加ライン数をインクリメント */
                            part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                            break ;
                        }
                    }
                    else
                    {
                        /* ライン数増加となる文字列(カンマ)を発見 かつ カンマカウントONの場合*/
                        if ( ( strstr(buf, cLine_count_str_tbl[i]) != NULL ) &&
                             ( sSearch_info.comma_cnt_flg == LINE_COUNT_ON ) )
                        {
                            ins_line_tmp++ ;    /* 追加ライン数をインクリメント */
                            part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                            break ;
                        }
                    }
                }
            }

            /* 追加ライン数に「１つのカウント開始／終了コメント間の追加ライン数」を加算 */
            ins_line = ins_line + ins_line_tmp ;
        }
    }

    return ins_line ;    /* 追加ライン数を返す */
}

/*******************************************************************************
* MODULE        : basic_cnt_chg
* ABSTRACT      : 基本カウント 変更ライン数カウント関数
* FUNCTION      : 基本カウント 変更ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：変更ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int basic_cnt_chg(FILE *fp)
{
    int chg_line ;             /* 変更ライン数 */
    int chg_line_tmp ;         /* １つのカウント開始／終了コメント間の変更ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    int str_pos ;              /* 空白以外の文字開始位置 */
    int count_start_flg ;      /* 部分カウント内カウント開始文字列発見フラグ */
    int i ;                    /* ループカウンタ */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    chg_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            count_start_flg = LINE_COUNT_OFF ;  /* カウント開始文字列発見フラグを初期化 */
            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].chg_cnt = 0 ;  /* 部分変更ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 変更ライン数が「0」の場合、「1」にする */
            /* ※if文の条件のみ追加などに対応するため */
            if ( ( part_line_tmp == 0 ) &&
                 ( count_start_flg == LINE_COUNT_ON ) )
            {
                part_line_tmp++ ;
            }

            /* 部分変更ライン数を退避 */
            sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント開始文字列発見フラグをON */
            count_start_flg = LINE_COUNT_ON ;

            /* １つのカウント開始／終了コメント間の変更ライン数を初期化 */
            chg_line_tmp = 0 ;

            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("変更のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    count_start_flg = LINE_COUNT_OFF ;  /* カウント開始文字列発見フラグを初期化 */
                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].chg_cnt = 0 ;  /* 部分変更ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 追加ライン数が「0」の場合、「1」にする */
                    /* ※if文の条件のみ追加などに対応するため */
                    if ( part_line_tmp == 0 )
                    {
                        part_line_tmp++ ;
                    }

                    /* 部分追加ライン数を退避 */
                    sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    /* 変更ライン数が「0」の場合、「1」にする */
                    /* ※if文の条件のみ変更などに対応するため */
                    if ( chg_line_tmp == 0 )
                    {
                        chg_line_tmp++ ;
                    }

                    break ;    /* 次のカウント開始文字列を検索する */
                }

                /* 空白以外の文字列先頭位置をチェック */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_posのインクリメントはfor文で */
                }

                /* 無効文字列（コメント行）and 「変更」によるコメントアウト部分カウントしない */
                /* ならば、次の行へ */
                if ( ( ( ( buf[str_pos] == '/' ) &&
                         ( buf[str_pos + 1 ] == '*' ) ) ||
                       ( ( buf[str_pos] == '/' ) &&
                         ( buf[str_pos + 1 ] == '/' ) ) ) &&
                     ( sSearch_info.chg_com_cnt_flg == LINE_COUNT_OFF ) )
                {
                    continue ;
                }

                /* ライン数カウント時に検索する文字列テーブル終端までループ */
                for ( i = 0; strcmp(cLine_count_str_tbl[i], "\0") != 0  ; i++ )
                {
                    /* ライン数カウント文字列テーブルが「カンマ」でない場合 */
                    if ( i != 0 )
                    {
                         /* ライン数増加となる文字列を発見 */
                        if ( strstr(buf, cLine_count_str_tbl[i]) != NULL )
                        {
                            chg_line_tmp++ ;    /* 変更ライン数をインクリメント */
                            part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                            break ;
                        }
                    }
                    else
                    {
                        /* ライン数増加となる文字列(カンマ)を発見 かつ カンマカウントONの場合*/
                        if ( ( strstr(buf, cLine_count_str_tbl[i]) != NULL ) &&
                             ( sSearch_info.comma_cnt_flg == LINE_COUNT_ON ) )
                        {
                            chg_line_tmp++ ;    /* 変更ライン数をインクリメント */
                            part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                            break ;
                        }
                    }
                }
            }

            /* 変更ライン数に「１つのカウント開始／終了コメント間の変更ライン数」を加算 */
            chg_line = chg_line + chg_line_tmp ;
        }
    }

    return chg_line ;    /* 変更ライン数を返す */
}

/*******************************************************************************
* MODULE        : basic_cnt_del
* ABSTRACT      : 基本カウント 削除ライン数カウント関数
* FUNCTION      : 基本カウント 削除ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：削除ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int basic_cnt_del(FILE *fp)
{
    int del_line ;             /* 削除ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    int i ;                    /* ループカウンタ */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    del_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].del_cnt = 0 ;  /* 部分削除ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分削除ライン数を退避 */
            sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("削除のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 無効部分開始文字列発見 */
                if ( strstr(buf, INVALID_START) != NULL )
                {
                    /* 無効部分終了文字列を見つけるまでループ */
                    while(1)
                    {
                        /* 文字列1行読み込み */
                        ret_p = fgets(buf,READ_SIZE,fp) ;

                        /* 無効部分終了文字列発見前にファイル読み込みが終了 */
                        if ( ret_p == NULL )
                        {
                            printf("\n----------------------------------------------------------------------\n") ;
                            printf("無効部分開始／終了のコメントが不正です\n") ;
                            return LINE_COUNT_NG ;
                        }

                        /* 無効部分終了文字列発見 */
                        if ( strstr(buf, INVALID_END) != NULL )
                        {
                            break ;    /* ループを抜ける */
                        }
                    }
                    continue ;    /* 次の行を読み込む */
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].del_cnt = 0 ;  /* 部分削除ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分削除ライン数を退避 */
                    sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }

                /* ライン数カウント時に検索する文字列テーブル終端までループ */
                for ( i = 0; strcmp(cLine_count_str_tbl[i], "\0") != 0  ; i++ )
                {
                    /* ライン数カウント文字列テーブルが「カンマ」でない場合 */
                    if ( i != 0 )
                    {
                         /* ライン数増加となる文字列を発見 */
                        if ( strstr(buf, cLine_count_str_tbl[i]) != NULL )
                        {
                            del_line++ ;        /* 削除ライン数をインクリメント */
                            part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                            break ;
                        }
                    }
                    else
                    {
                        /* ライン数増加となる文字列(カンマ)を発見 かつ カンマカウントONの場合*/
                        if ( ( strstr(buf, cLine_count_str_tbl[i]) != NULL ) &&
                             ( sSearch_info.comma_cnt_flg == LINE_COUNT_ON ) )
                        {
                            del_line++ ;        /* 削除ライン数をインクリメント */
                            part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                            break ;
                        }
                    }
                }
            }
        }
    }

    return del_line ;    /* 削除ライン数を返す */
}

/*******************************************************************************
* MODULE        : line_cnt_ins
* ABSTRACT      : ライン数そのままカウント 追加ライン数カウント関数
* FUNCTION      : ライン数そのままカウント 追加ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：追加ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int line_cnt_ins(FILE *fp)
{
    int ins_line ;             /* 追加ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    int str_pos ;              /* 空白以外の文字開始位置 */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    ins_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].ins_cnt = 0 ;  /* 部分追加ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分追加ライン数を退避 */
            sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("追加のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].ins_cnt = 0 ;  /* 部分追加ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分追加ライン数を退避 */
                    sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }

                /* 空白以外の文字列先頭位置をチェック */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_posのインクリメントはfor文で */
                }

                /* 無効文字列（コメント行） or 空白のみの行なら次の行へ */
                if ( ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '*' ) ) ||
                     ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '/' ) ) ||
                     ( buf[str_pos] == '\n' ) )
                {
                    continue ;
                }

                /* 改行以外は追加ライン数をインクリメント */
                if ( buf[0] != '\n' )
                {
                    ins_line++ ;        /* 追加ライン数をインクリメント */
                    part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                }
            }
        }
    }

    return ins_line ;    /* 追加ライン数を返す */
}

/*******************************************************************************
* MODULE        : line_cnt_chg
* ABSTRACT      : ライン数そのままカウント 変更ライン数カウント関数
* FUNCTION      : ライン数そのままカウント 変更ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：変更ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int line_cnt_chg(FILE *fp)
{
    int chg_line ;             /* 変更ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    int str_pos ;              /* 空白以外の文字開始位置 */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    chg_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].chg_cnt = 0 ;  /* 部分変更ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分変更ライン数を退避 */
            sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("変更のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].chg_cnt = 0 ;  /* 部分変更ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分追加ライン数を退避 */
                    sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }

                /* 空白以外の文字列先頭位置をチェック */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_posのインクリメントはfor文で */
                }

                /* (無効文字列（コメント行）and 「変更」によるコメントアウト部分カウントしない) */
                /*  or 空白のみの行なら次の行へ */
                if ( ( ( ( ( buf[str_pos] == '/' ) &&
                           ( buf[str_pos + 1 ] == '*' ) ) ||
                         ( ( buf[str_pos] == '/' ) &&
                           ( buf[str_pos + 1 ] == '/' ) ) ) &&
                         ( sSearch_info.chg_com_cnt_flg == LINE_COUNT_OFF ) ) ||
                     ( buf[str_pos] == '\n' ) )
                {
                    continue ;
                }

                /* 改行以外は変更ライン数をインクリメント */
                if ( buf[0] != '\n' )
                {
                    chg_line++ ;        /* 変更ライン数をインクリメント */
                    part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                }
            }
        }
    }

    return chg_line ;    /* 変更ライン数を返す */
}

/*******************************************************************************
* MODULE        : line_cnt_del
* ABSTRACT      : ライン数そのままカウント 削除ライン数カウント関数
* FUNCTION      : ライン数そのままカウント 削除ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：削除ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int line_cnt_del(FILE *fp)
{
    int del_line ;             /* 削除ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    int str_pos ;              /* 空白以外の文字開始位置 */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    del_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].del_cnt = 0 ;  /* 部分削除ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分削除ライン数を退避 */
            sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("削除のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 無効部分開始文字列発見 */
                if ( strstr(buf, INVALID_START) != NULL )
                {
                    /* 無効部分終了文字列を見つけるまでループ */
                    while(1)
                    {
                        /* 文字列1行読み込み */
                        ret_p = fgets(buf,READ_SIZE,fp) ;

                        /* 無効部分終了文字列発見前にファイル読み込みが終了 */
                        if ( ret_p == NULL )
                        {
                            printf("\n----------------------------------------------------------------------\n") ;
                            printf("無効部分開始／終了のコメントが不正です\n") ;
                            return LINE_COUNT_NG ;
                        }

                        /* 無効部分終了文字列発見 */
                        if ( strstr(buf, INVALID_END) != NULL )
                        {
                            break ;    /* ループを抜ける */
                        }
                    }
                    continue ;    /* 次の行を読み込む */
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].del_cnt = 0 ;  /* 部分削除ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分削除ライン数を退避 */
                    sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }

                /* 空白以外の文字列先頭位置をチェック */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_posのインクリメントはfor文で */
                }

                /* 空白のみの行なら次の行へ */
                if ( buf[str_pos] == '\n' )
                {
                    continue ;
                }

                /* 改行以外は削除ライン数をインクリメント */
                if ( buf[0] != '\n' )
                {
                    del_line++ ;        /* 削除ライン数をインクリメント */
                    part_line_tmp++ ;   /* 部分カウントライン数をインクリメント */
                }
            }
        }
    }

    return del_line ;    /* 削除ライン数を返す */
}

/*******************************************************************************
* MODULE        : increase_cnt_ins
* ABSTRACT      : ひたすらカウント 追加ライン数カウント関数
* FUNCTION      : ひたすらカウント 追加ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：追加ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int increase_cnt_ins(FILE *fp)
{
    int ins_line ;             /* 追加ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    ins_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].ins_cnt = 0 ;  /* 部分追加ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分追加ライン数を退避 */
            sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("追加のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].ins_cnt = 0 ;  /* 部分追加ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分追加ライン数を退避 */
                    sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }
                /* 1行読み込み毎に追加ライン数、部分カウントライン数をインクリメント */
                else
                {
                    ins_line++ ;
                    part_line_tmp++ ;
                }
            }
        }
    }

    return ins_line ;    /* 追加ライン数を返す */
}

/*******************************************************************************
* MODULE        : increase_cnt_chg
* ABSTRACT      : ひたすらカウント 変更ライン数カウント関数
* FUNCTION      : ひたすらカウント 変更ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：変更ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int increase_cnt_chg(FILE *fp)
{
    int chg_line ;             /* 変更ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    chg_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].chg_cnt = 0 ;  /* 部分変更ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分変更ライン数を退避 */
            sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("変更のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].chg_cnt = 0 ;  /* 部分変更ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分追加ライン数を退避 */
                    sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }
                /* 1行読み込み毎に変更ライン数、部分カウントライン数をインクリメント */
                else
                {
                    chg_line++ ;
                    part_line_tmp++ ;
                }
            }
        }
    }

    return chg_line ;    /* 変更ライン数を返す */
}

/*******************************************************************************
* MODULE        : increase_cnt_del
* ABSTRACT      : ひたすらカウント 削除ライン数カウント関数
* FUNCTION      : ひたすらカウント 削除ライン数カウント
* NOTE          : 第1引数：読み込むファイルポインタ(IN)
* RETURN        : 正常終了：削除ライン数
*                 異常終了：LINE_COUNT_NG
*******************************************************************************/
int increase_cnt_del(FILE *fp)
{
    int del_line ;             /* 削除ライン数 */
    int part_line_tmp ;        /* 部分カウントライン数 */
    char buf[READ_SIZE + 1] ;  /* ファイル読み込みバッファ */
    char *ret_p ;              /* fgetsの戻り値 */

    /* 初期化 */
    del_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* 部分カウント実行回数初期化 */
    sPart_count_no = 0 ;

    /* ファイル終端まで文字列を1行ずつ読み込む */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* 部分カウント開始文字列発見 */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* 部分カウント可能数オーバー */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("部分カウント可能数をオーバーしています\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].del_cnt = 0 ;  /* 部分削除ライン数を初期化 */
        }
        /* 部分カウント終了文字列発見 */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* 部分削除ライン数を退避 */
            sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* 次の部分カウントを開始 */
        }

        /* カウント開始文字列発見 */
        if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* カウント終了文字列を見つけるまでループ */
            while(1)
            {
                /* 文字列1行読み込み */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* カウント終了文字列発見前にファイル読み込みが終了 */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("削除のカウント開始／終了のコメントが不正です\n") ;
                    return LINE_COUNT_NG ;
                }

                /* 部分カウント開始文字列発見 */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* 部分カウント可能数オーバー */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("部分カウント可能数をオーバーしています\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].del_cnt = 0 ;  /* 部分削除ライン数を初期化 */
                }
                /* 部分カウント終了文字列発見 */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* 部分削除ライン数を退避 */
                    sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* 次の部分カウントを開始 */
                }

                /* カウント終了文字列発見 */
                if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* 次のカウント開始文字列を検索する */
                }
                /* 1行読み込み毎に削除ライン数、部分カウントライン数をインクリメント */
                else
                {
                    del_line++ ;
                    part_line_tmp++ ;
                }
            }
        }
    }

    return del_line ;    /* 削除ライン数を返す */
}

/*******************************************************************************
* MODULE        : cnt_explain
* ABSTRACT      : カウント方法説明関数
* FUNCTION      : カウント方法説明をするだけ
* NOTE          : 
* RETURN        : なし
*******************************************************************************/
void cnt_explain(void)
{
    printf("\n----------------------------------------------------------------------\n") ;
    puts("■基本カウント") ;
    puts("  ・以下の文字列を行内に発見すると、1ラインと数える") ;
    puts("    ⇒「;」「if」「else if」「else」「switch」「case」「default」") ;
    puts("      「for」「while」「#define」") ;
    puts("  ・条件文は条件判定のライン数によらず1ラインとなる") ;
    puts("  ※「,」のライン数カウントは最初に選択したカウント方法による") ;

    puts("\n■ライン数そのままカウント") ;
    puts("  ・「空白のみ」or「改行のみ」以外のライン数を1ラインと数える") ;
    puts("  ・条件文は条件判定のライン数分をカウントする") ;
    puts("  ・「{」「}」のみの行も1ラインにカウントされる") ;

    puts("\n■ひたすらカウント") ;
    puts("  ・とりあえず多くなるように数えてみる") ;
}

