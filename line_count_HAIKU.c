/*-----------------------------------------------
*   �C���N���[�h�t�@�C��
*------------------------------------------------
*/
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

/*-----------------------------------------------
*   �萔��`
*------------------------------------------------
*/
#define COMMENT_MAX       101    /* �J�E���g�J�n�^�I��������ő咷 */
#define FNAME_MAX         101    /* �ǂݍ��ރt�@�C�����ő咷 */
#define PART_COUNT_MAX    100    /* �����J�E���g�\�ő吔 */
#define READ_SIZE         300    /* 1�s�̕������ő咷 */
#define LINE_COUNT_OK     0
#define LINE_COUNT_NG    -1
#define LINE_COUNT_ON     1
#define LINE_COUNT_OFF    0

#define PART_COUNT_START  "Part -s"   /* �����J�E���g�J�n�L�[���[�h */
#define PART_COUNT_END    "Part -e"   /* �����J�E���g�I���L�[���[�h */
#define INVALID_START     "Skip -s"   /* ���������J�n�L�[���[�h */
#define INVALID_END       "Skip -e"   /* ���������I���L�[���[�h */

/*-----------------------------------------------
*   �\���̒�`
*------------------------------------------------
*/
/* �J�E���g�J�n�^�I��������\���� */
typedef struct {
    unsigned char key_word[COMMENT_MAX] ;   /* �J�E���g�J�n�^�I���L�[���[�h */
    unsigned char ins_key[COMMENT_MAX] ;    /* �ǉ��������L�[���[�h */
    unsigned char chg_key[COMMENT_MAX] ;    /* �ύX�������L�[���[�h */
    unsigned char del_key[COMMENT_MAX] ;    /* �폜�������L�[���[�h */
    unsigned char start_key[COMMENT_MAX] ;  /* �J�n�������L�[���[�h */
    unsigned char end_key[COMMENT_MAX] ;    /* �I���������L�[���[�h */
    short         chg_com_cnt_flg ;         /* �u�ύX�v���C�����J�E���g ���������J�E���g�t���O */
    short         comma_cnt_flg ;           /* �J���}�J�E���g�L���^�����t���O */
} _SEARCH_INFO ;

/* �ǂݍ��݃t�@�C�����ێ��\���� */
typedef struct {
    char fname[FNAME_MAX] ;
} _SEARCH_FILEINFO ;

/* �����J�E���g���C�����ێ��\���� */
typedef struct {
    int ins_cnt ;    /* �ǉ����C���� */
    int chg_cnt ;    /* �ύX���C���� */
    int del_cnt ;    /* �폜���C���� */
} _PART_COUNT ;

/*-----------------------------------------------
*   �ϐ���`
*------------------------------------------------
*/
_SEARCH_INFO      sSearch_info ;    /* �J�E���g�J�n�^�I��������\���� */
_SEARCH_FILEINFO *sSearch_fname ;   /* �ǂݍ��݃t�@�C�����ێ��\���� */
_PART_COUNT       sPart_count[PART_COUNT_MAX] ;  /* �����J�E���g���C�����ێ��\���� */

int      sPart_count_no ;    /* �����J�E���g���s�� */

/*-----------------------------------------------
*   �e�[�u����`
*------------------------------------------------
*/
/* ���C�����J�E���g���Ɍ������镶���� */
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
    "\0"    /* �e�[�u���I�[ */
} ;

/* �ǂݍ��ݑΏۂƂȂ�t�@�C�� */
const char *cLine_count_read_tbl[] = {
    ".txt",
    ".c",
    ".h",
    "\0"    /* �e�[�u���I�[ */
} ;

/*-----------------------------------------------
*  �v���g�^�C�v�錾
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
* ABSTRACT      : ���C���֐�
* FUNCTION      : �N�����ɃR�[�������
* NOTE          : 
* RETURN        : �Ȃ�
*******************************************************************************/
void main(void)
{
    int           menu_no ;  /* ���͂��ꂽ���l */
    int           read_flg ; /* �t�@�C���L���t���O */
    DIR           *dir ;     /* �f�B���N�g���X�g���[�� */
    struct dirent *dir_p ;   /* �f�B���N�g���G���g�� */
    int           i ;        /* ���[�v�J�E���^ */

    /* �O���ϐ������� */
    sSearch_fname = NULL ;

    /* �N�����[�v */
    while(1)
    {
        printf("\n----------------------------------------------------------------------\n") ;
        printf("�|���C�����J�E���g�|\n\n") ;

        printf("���u*.c�v�u*.cpp�v�u*.h�v�u*.txt�v�̂݌����ΏۂƂȂ�܂�\n") ;
        printf("���ǂݍ��ރt�@�C���̃t�@�C�������͔��p100�����ȉ��Ƃ��Ă�������\n\n") ;
        printf("���C�������J�E���g����t�@�C�����J�����g�f�B���N�g���ɒu���Ă�������\n") ;
        printf("�u���܂������H\n\n") ;
        printf("1�D�͂�  2�D��߂܂�\n");
        printf(">") ;

        scanf("%d",&menu_no) ;
        scanf("%*c") ;    /* �o�b�t�@�N���A */

        /* �t�@�C���L���t���O������ */
        read_flg = LINE_COUNT_OFF ;

        switch(menu_no)
        {
            case 1:
                /* �J�����g�f�B���N�g���I�[�v�� */
                dir = opendir(".") ;

                /* �f�B���N�g�����t�@�C���ǂݍ��� */
                while ( (dir_p = readdir(dir)) != NULL )
                {
                    /* �ǂݍ��ݑΏۂƂȂ�t�@�C���e�[�u���I�[�܂Ō������[�v */
                    for ( i = 0; strcmp(cLine_count_read_tbl[i], "\0") != 0  ; i++ )
                    {
                        /* �u*.txt�v�u*.c�v�u*.h�v������΁A�J�E���g���j���[�� */
                        if ( strstr(dir_p->d_name, cLine_count_read_tbl[i]) != NULL )
                        {
                            read_flg = LINE_COUNT_ON ;    /* �t�@�C���L���t���OON */
                            break ;
                        }
                    }
                    /* �J�E���g�Ώۃt���O�����݂�����t�@�C���ǂݍ��݃��[�v�𔲂��� */
                    if ( read_flg == LINE_COUNT_ON )
                    {
                        break ;
                    }
                }

                /* �J�����g�f�B���N�g���ɓǂݍ��ރt�@�C��������ꍇ */
                if ( read_flg == LINE_COUNT_ON )
                {
                    count_menu() ;    /* �J�E���g���j���[�� */
                }
                /* �t�@�C�����Ȃ���΋N�����j���[�� */
                else
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("���u���ɂȂ����t�@�C���͌��ݑ��݂��Ă���܂���\n") ;
                    printf("������x���u���ɂȂ����f�B���N�g�������m���߂̏�A�Ď��s���Ă�������\n\n") ;
                }
                break;

            case 2:
                printf("Good Night\n") ;   /* HOTEI PDF�̏I����ʂ�\�� */
                sleep(1) ;
                /* �m�ۂ��Ă����������̉�� */
                if ( sSearch_fname != NULL )
                {
                    free(sSearch_fname) ;
                }
                return ;    /* �I�� */

            default:
                printf("\n----------------------------------------------------------------------\n") ;
                printf("���̓G���[�ł�(�����͔��p�œ���)\n\n");
        }
    }
}

/*******************************************************************************
* MODULE        : count_menu
* ABSTRACT      : �J�E���g���j���[�֐�
* FUNCTION      : ���j���[�I���֐�
* NOTE          : 
* RETURN        : �Ȃ�
*******************************************************************************/
void count_menu(void)
{
    int    menu_no ;    /* ���͂��ꂽ���l */
    int    read_cnt ;   /* �߂�l�i����I�����͓ǂݍ��ރt�@�C����) */

    /* ���j���[�I�����[�v */
    while(1)
    {
        printf("\n----------------------------------------------------------------------\n") ;
        printf("���C�������J�E���g������@��I�����Ă�������\n\n");
        printf("1�D��{�J�E���g  2�D���C�������̂܂܃J�E���g  3�D�Ђ�����J�E���g\n") ;
        printf("4. �J�E���g���@�̐���\n") ;
        printf(">") ;

        scanf("%d",&menu_no);
        scanf("%*c") ;    /* �o�b�t�@�N���A */

        /* �J�����g�f�B���N�g�����S�t�@�C�����擾 */
        read_cnt = get_filename() ;

        /* �t�@�C���ǂݍ��ݐ��� */
        if ( read_cnt != LINE_COUNT_NG )
        {
            switch(menu_no)
            {
                case 1:
                    /* break�s�v */
                case 2:
                    /* break�s�v */
                case 3:
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("���ǂݍ��ރt�@�C���́u%s�v�ł�\n",sSearch_fname[0].fname) ;

                    /* �J�E���g�J�n�^�I��������ݒ� */
                    serch_com_set() ;

                    /* �J�E���g���s */
                    count_line(menu_no,read_cnt) ;
                    return ;

                case 4:
                    cnt_explain() ;
                    break ;

                default:
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("���̓G���[�ł�(�����͔��p�œ���)\n\n") ;
            }
        }
        /* �t�@�C���ǂݍ��ݎ��s�i�ēx�N�����j���[�ցj */
        else
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("�t�@�C���ǂݍ��ݎ��s���܂���\n\n") ;
            return ;
        }
    }
}

/*******************************************************************************
* MODULE        : count_line
* ABSTRACT      : ���C�����J�E���g���s�֐�
* FUNCTION      : ���C�����J�E���g���s
* NOTE          : ��1�����F�I�������J�E���g���@(IN)
*                 ��2�����F�ǂݍ��ރt�@�C����  (IN)
* RETURN        : �Ȃ�
*******************************************************************************/
void count_line(int menu_no,
                int read_cnt)
{
    FILE *fp ;            /* �t�@�C���|�C���^ */
    int i ;               /* ���[�v�J�E���^ */
    int j ;               /* ���[�v�J�E���^ */
    int ins_line ;        /* �ǉ����C���� */
    int chg_line ;        /* �ύX���C���� */
    int del_line ;        /* �폜���C���� */
    int continue_menu ;   /* �J�E���g���s���j���[�I��ԍ� */

    /* �ǂݍ��ރt�@�C���������[�v */
    for ( i = 0; i < read_cnt; i++ )
    {
        /* �����J�E���g���C�����\���̏����� */
        for ( j = 0; j < PART_COUNT_MAX; j++ )
        {
            memset(&sPart_count[j], -1, sizeof(_PART_COUNT)) ;
        }

        /* ����ǂݍ��݃t�@�C���ȍ~�͎��̃t�@�C�����J�E���g���邩�m�F */
        if ( i != 0 )
        {
            /* �J�E���g���s�₢���킹���[�v */
            while(1)
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����ɓǂݍ��ރt�@�C���́u%s�v�ł�\n",sSearch_fname[i].fname) ;
                printf("  �J�E���g���s���܂����H\n\n") ;
                printf("1�D�͂�  2�D������  3.�J�E���g�J�n�^�I���������ύX���đ��s\n") ;
                printf(">");

                scanf("%d",&continue_menu);
                scanf("%*c") ;    /* �o�b�t�@�N���A */

                if ( continue_menu == 1 )
                {
                    break ;    /* ���[�v������ */
                }
                else if ( continue_menu == 2 )
                {
                    return ;   /* �ēx�A�������j���[�� */
                }
                else if ( continue_menu == 3 )
                {
                    serch_com_set() ;    /* �J�E���g�J�n�^�I��������ݒ� */
                    break ;              /* ���[�v������ */
                }
                else
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("���̓G���[�ł�(�����͔��p�œ���)\n\n") ;
                }
            }
        }

        /* �t�@�C���I�[�v�� */
        fp = fopen(sSearch_fname[i].fname,"r") ;
        /* �I�[�v�����s */
        if ( fp == NULL )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("�t�@�C���I�[�v�����s���܂���\n") ;
            return ;
        }

        /* ���ꂼ��̃J�E���g���@�̊֐����R�[�� */
        switch(menu_no)
        {
            /* ��{�J�E���g */
            case 1:
                ins_line = basic_cnt_ins(fp) ;  /* �ǉ����C�����J�E���g */
                fseek(fp,  0, SEEK_SET) ;       /* �t�@�C���|�C���^��擪�ɖ߂� */

                chg_line = basic_cnt_chg(fp) ;  /* �ύX���C�����J�E���g */
                fseek(fp,  0, SEEK_SET) ;       /* �t�@�C���|�C���^��擪�ɖ߂� */

                del_line = basic_cnt_del(fp) ;  /* �폜���C�����J�E���g */
                break ;

            /* ���C�������̂܂܃J�E���g */
            case 2:
                ins_line = line_cnt_ins(fp) ;   /* �ǉ����C�����J�E���g */
                fseek(fp,  0, SEEK_SET) ;       /* �t�@�C���|�C���^��擪�ɖ߂� */

                chg_line = line_cnt_chg(fp) ;   /* �ύX���C�����J�E���g */
                fseek(fp,  0, SEEK_SET) ;       /* �t�@�C���|�C���^��擪�ɖ߂� */

                del_line = line_cnt_del(fp) ;   /* �폜���C�����J�E���g */
                break ;

            /* �Ђ�����J�E���g */
            case 3:
                ins_line = increase_cnt_ins(fp) ;   /* �ǉ����C�����J�E���g */
                fseek(fp,  0, SEEK_SET) ;           /* �t�@�C���|�C���^��擪�ɖ߂� */

                chg_line = increase_cnt_chg(fp) ;   /* �ύX���C�����J�E���g */
                fseek(fp,  0, SEEK_SET) ;           /* �t�@�C���|�C���^��擪�ɖ߂� */

                del_line = increase_cnt_del(fp) ;   /* �폜���C�����J�E���g */
                break ;

            default:
                return ;    /* �ʂ蓾�Ȃ� */
        }

        /* �t�@�C���N���[�Y */
        fclose(fp) ;

        /* �J�E���g�J�n�^�I���̃R�����g���s���������ꍇ */
        if ( ( ins_line == LINE_COUNT_NG ) ||
             ( chg_line == LINE_COUNT_NG ) ||
             ( del_line == LINE_COUNT_NG ) )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("�J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
            printf("�������j���[�֖߂�܂�\n") ;
            return ;
        }

        /* �J�E���g���C�����̕\�� */
        printf("\n----------------------------------------------------------------------\n") ;
        printf("���u%s�v �̒ǉ��^�ύX�^�폜���C����\n",sSearch_fname[i].fname) ;
        printf("   �ǉ��F%dL\n",ins_line) ;
        printf("   �ύX�F%dL\n",chg_line) ;
        printf("   �폜�F%dL\n",del_line) ;

        /* �����J�E���g���C�����̕\�� */
        for ( j = 0; j < sPart_count_no; j++ )
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("���uPart-Count-%d�v �̒ǉ��^�ύX�^�폜���C����\n",j + 1) ;
            printf("   �ǉ��F%3dL",sPart_count[j].ins_cnt) ;
            printf("   �ύX�F%3dL",sPart_count[j].chg_cnt) ;
            printf("   �폜�F%3dL",sPart_count[j].del_cnt) ;
        }
        printf("\n----------------------------------------------------------------------\n\n") ;
    }
}

/*******************************************************************************
* MODULE        : serch_com_set
* ABSTRACT      : �J�E���g�J�n�^�I��������ݒ�֐�
* FUNCTION      : �J�E���g�J�n�^�I����������O���ϐ��ɕێ�����
* NOTE          : 
* RETURN        : �Ȃ�
*******************************************************************************/
void serch_com_set(void)
{
    int  menu_no ;             /* ���͂��ꂽ���l */
    int  i ;                   /* ���[�v�J�E���^ */
    FILE *fp ;                 /* �t�@�C���|�C���^ */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    while(1)
    {
        /* �J�E���g�J�n�^�I��������\���̏����� */
        memset(&sSearch_info, 0x00, sizeof(_SEARCH_INFO)) ;
        /* �o�b�t�@������ */
        memset(buf, '\0', sizeof(buf)) ;

        /* ���j���[�I�����[�v */
        while(1)
        {
            printf("\n----------------------------------------------------------------------\n") ;
            printf("���C�����J�E���g�J�n�^�I���L�[���[�h��ݒ肵�܂�\n\n") ;
            printf("1�D�t�@�C���isearch_info.ini�j����ǂݍ���  2�D���ړ���\n") ;
            printf(">");

            scanf("%d",&menu_no);
            scanf("%*c") ;    /* �o�b�t�@�N���A */

            /* �t�@�C������ǂݍ��� */
            if ( menu_no == 1 )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�usearch_info.ini�v���ȉ��̓��e�ɂȂ��Ă��邱�Ƃ��m�F���Ă�������\n") ;
                printf("��1�s�ځF�J�E���g�J�n�^�I���L�[���[�h�i�⏈�ԍ��Ȃǁj\n") ;
                printf("��2�s�ځF�ǉ��������L�[���[�h        �iINS �Ȃǁj\n") ;
                printf("��3�s�ځF�ύX�������L�[���[�h        �iCHG �Ȃǁj\n") ;
                printf("��4�s�ځF�폜�������L�[���[�h        �iDEL �Ȃǁj\n") ;
                printf("��5�s�ځF�J�E���g�J�n�������L�[���[�h�i-S  �Ȃǁj\n") ;
                printf("��6�s�ځF�J�E���g�I���������L�[���[�h�i-E  �Ȃǁj\n") ;
                printf("��7�s�ځF�u�ύX�v�ɂ��R�����g�A�E�g���ꂽ�������J�E���g����iON or OFF�j\n") ;
                printf("��8�s�ځF�u,�v�̑��݂���s��1���C���ƃJ�E���g����iON or OFF�j\n\n") ;

                printf("�m�F���܂������H\n\n") ;
                printf("1�D�͂�  2�D�m�F�����Ă܂�\n");
                printf(">") ;

                scanf("%d",&menu_no) ;
                scanf("%*c") ;    /* �o�b�t�@�N���A */

                switch(menu_no)
                {
                    case 1:
                        break ;    /* �������s */

                    case 2:
                        continue ; /* �ēx�m�F */

                    default:
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("���̓G���[�ł�(�����͔��p�œ���)\n\n") ;
                        continue ; /* �ēx�m�F */
                }

                /* �J�E���g�J�n�^�I��������ݒ�t�@�C���I�[�v�� */
                fp = fopen("search_info.ini","r") ;

                /* �I�[�v�����s */
                if ( fp == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�t�@�C���I�[�v�����s���܂���\n") ;
                    printf("search_info.ini���J�����g�f�B���N�g���ɒu���Ă�������\n") ;
                    continue ;
                }

                /* �������1�s���ǂݍ��݁A�J�E���g�J�n�^�I��������\���̂ɃR�s�[ */
                fgets(buf,READ_SIZE,fp) ;     /* 1�s�ǂݍ��� */
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                strncpy(sSearch_info.key_word, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                strncpy(sSearch_info.ins_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                strncpy(sSearch_info.chg_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                strncpy(sSearch_info.del_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                strncpy(sSearch_info.start_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                strncpy(sSearch_info.end_key, buf, COMMENT_MAX) ;

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                if ( strcmp(buf, "ON" ) == 0 )
                {
                    sSearch_info.chg_com_cnt_flg = LINE_COUNT_ON ;  /* �u�ύX�v�̃R�����g�A�E�g�������J�E���g���� */
                }
                else
                {
                    sSearch_info.chg_com_cnt_flg = LINE_COUNT_OFF ; /* �u�ύX�v�̃R�����g�A�E�g�����̓J�E���g���Ȃ� */
                }

                memset(buf, '\0', sizeof(buf)) ;
                fgets(buf,READ_SIZE,fp) ;
                buf[strlen(buf) - 1] = '\0' ; /* ���s���I�[�����ɕϊ� */
                if ( strcmp(buf, "ON" ) == 0 )
                {
                    sSearch_info.comma_cnt_flg = LINE_COUNT_ON ;  /* �u�J���}�v��L�����C���Ƃ��ăJ�E���g���� */
                }
                else
                {
                    sSearch_info.comma_cnt_flg = LINE_COUNT_OFF ; /* �u�J���}�v��L�����C���Ƃ��ăJ�E���g���Ȃ� */
                }

                /* �t�@�C���N���[�Y */
                fclose(fp) ;
                break ;
            }
            /* ���ړ��� */
            else if ( menu_no == 2 )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�J�E���g�J�n�^�I���L�[���[�h�i�⏈�ԍ��Ȃǁj����͂��Ă�������\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.key_word) ;  /* �u�J�E���g�J�n�^�I���ő啶���� + 1�v�܂Ŋi�[ */
                scanf("%*c") ;                               /* �o�b�t�@�N���A */

                printf("�ǉ��������L�[���[�h(INS �Ȃ�)����͂��Ă�������\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.ins_key) ;
                scanf("%*c") ;

                printf("�ύX�������L�[���[�h(CHG �Ȃ�)����͂��Ă�������\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.chg_key) ;
                scanf("%*c") ;

                printf("�폜�������L�[���[�h(DEL �Ȃ�)����͂��Ă�������\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.del_key) ;
                scanf("%*c") ;

                printf("�J�E���g�J�n�������L�[���[�h(-S �Ȃ�)����͂��Ă�������\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.start_key) ;
                scanf("%*c") ;

                printf("�J�E���g�I���������L�[���[�h(-E �Ȃ�)����͂��Ă�������\n") ;
                printf(">") ;
                scanf("%101[^\n]", sSearch_info.end_key) ;
                scanf("%*c") ;

                /* ���j���[�I�����[�v */
                while(1)
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�u�ύX�v�̃J�E���g���@��I�����Ă�������\n\n") ;
                    printf("1�D�u�ύX�v�ɂ��A�R�����g�A�E�g���������̓J�E���g���Ȃ�\n") ;
                    printf("2�D�u�ύX�v�ɂ��A�R�����g�A�E�g�������܂߂ăJ�E���g����\n") ;
                    printf(">") ;

                    scanf("%d",&menu_no);
                    scanf("%*c") ;    /* �o�b�t�@�N���A */

                    if ( menu_no == 1 )
                    {
                        sSearch_info.chg_com_cnt_flg = LINE_COUNT_OFF ;  /* �u�ύX�v�̃R�����g�A�E�g�����̓J�E���g���Ȃ� */
                        break ;
                    }
                    else if ( menu_no == 2 )
                    {
                        sSearch_info.chg_com_cnt_flg = LINE_COUNT_ON ;   /* �u�ύX�v�̃R�����g�A�E�g�������J�E���g���� */
                        break ;
                    }
                    else
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("���̓G���[�ł�(�����͔��p�œ���)\n\n") ;
                    }
                }

                /* ���j���[�I�����[�v */
                while(1)
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�u�J���}�v�̃J�E���g���@��I�����Ă�������\n\n") ;
                    printf("1�D�u,�v�����݂��Ă��L�����C���Ƃ��ăJ�E���g���Ȃ�\n") ;
                    printf("2�D�u,�v�̑��݂���s��1���C���ƃJ�E���g����\n") ;
                    printf(">") ;

                    scanf("%d",&menu_no);
                    scanf("%*c") ;    /* �o�b�t�@�N���A */

                    if ( menu_no == 1 )
                    {
                        sSearch_info.comma_cnt_flg = LINE_COUNT_OFF ; /* �u�J���}�v��L�����C���Ƃ��ăJ�E���g���Ȃ� */
                        break ;
                    }
                    else if ( menu_no == 2 )
                    {
                        sSearch_info.comma_cnt_flg = LINE_COUNT_ON ;  /* �u�J���}�v��L�����C���Ƃ��ăJ�E���g���� */
                        break ;
                    }
                    else
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("���̓G���[�ł�(�����͔��p�œ���)\n\n") ;
                    }
                }

                break ;
            }
            else
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("���̓G���[�ł�(�����͔��p�œ���)\n\n") ;
            }
        }

        /* �J�E���g�J�n�^�I���s���`�F�b�N */
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
            printf("�J�E���g�J�n�^�I���������s���ł��B�ēx���������͂��Ă�������\n\n") ;
        }
        /* �J�E���g�J�n�^�I�������񂪐��� */
        else
        {
            break ;    /* �֐������� */
        }
    }
}

/*******************************************************************************
* MODULE        : get_filename
* ABSTRACT      : �ǂݍ��݃t�@�C�����擾�֐�
* FUNCTION      : �ǂݍ��ރt�@�C�������O���ϐ��ɕێ�����
* NOTE          : 
* RETURN        : ����I���F�ǂݍ��ރt�@�C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int get_filename(void)
{
    DIR            *dir ;       /* �f�B���N�g���X�g���[�� */
    struct dirent  *dir_p ;     /* �f�B���N�g���G���g�� */
    int            read_cnt ;   /* �ǂݍ��ރt�@�C���� */
    int            i ;          /* ���[�v�J�E���^ */

    /* ������ */
    read_cnt = 0 ;

    /* �J�����g�f�B���N�g���I�[�v�� */
    dir = opendir(".") ;

    /* �ǂݍ��ރt�@�C�����`�F�b�N */
    while ( (dir_p = readdir(dir)) != NULL )
    {
        /* �ǂݍ��ݑΏۂƂȂ�t�@�C���e�[�u���I�[�܂Ō������[�v */
        for ( i = 0; strcmp(cLine_count_read_tbl[i], "\0") != 0  ; i++ )
        {
            /* �u*.txt�v�u*.c�v�u*.h�v������΁A�ǂݍ��ރt�@�C�������C���N�������g */
            if ( strstr(dir_p->d_name, cLine_count_read_tbl[i]) != NULL )
            {
                read_cnt++ ;
                break ;
            }
        }
    }
    /* �J�����g�f�B���N�g���N���[�Y */
    closedir(dir) ;

    /* �ǂݍ��ރt�@�C�����Ȃ���Έُ�I�� */
    if ( read_cnt == 0 )
    {
        return LINE_COUNT_NG ;
    }

    /* �O���ϐ��̃�������� */
    if ( sSearch_fname != NULL )
    {
        free(sSearch_fname) ;
        sSearch_fname = NULL ;
    }

    /* �ǂݍ��ރt�@�C�������̃������m�� */
    sSearch_fname = malloc(sizeof(_SEARCH_FILEINFO) * read_cnt) ;

    /* ������ */
    memset(sSearch_fname, '\0', sizeof(sSearch_fname)) ;
    read_cnt = 0 ;

    /* �J�����g�f�B���N�g���I�[�v�� */
    dir = opendir(".") ;

    /* �ǂݍ��ރt�@�C�������O���ϐ��ɕێ� */
    while ( (dir_p = readdir(dir)) != NULL )
    {
        /* �ǂݍ��ݑΏۂƂȂ�t�@�C���e�[�u���I�[�܂Ō������[�v */
        for ( i = 0; strcmp(cLine_count_read_tbl[i], "\0") != 0  ; i++ )
        {
            /* �ǂݍ��ݑΏۂ̃t�@�C���𔭌� */
            if ( strstr(dir_p->d_name, cLine_count_read_tbl[i]) != NULL )
            {
                /* �ő�t�@�C���������z�����t�@�C���̓t�@�C�����ێ����Ȃ� */
                if ( strlen(dir_p->d_name) >= FNAME_MAX )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ǂݍ��ރt�@�C���̍ő�t�@�C���������z�����t�@�C��������܂��B�ǂݎ��X�L�b�v���܂�\n\n") ;
                }
                else
                {
                    strcpy(sSearch_fname[read_cnt].fname, dir_p->d_name) ;    /* �ǂݍ��ރt�@�C�������R�s�[ */
                    read_cnt++ ;    /* �\���̂̓Y�������C���N�������g */
                    break ;
                }
            }
        }
    }
    /* �J�����g�f�B���N�g���N���[�Y */
    closedir(dir) ;

    return read_cnt ;    /* �ǂݍ��ރt�@�C������Ԃ� */
}

/*******************************************************************************
* MODULE        : basic_cnt_ins
* ABSTRACT      : ��{�J�E���g �ǉ����C�����J�E���g�֐�
* FUNCTION      : ��{�J�E���g �ǉ����C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�ǉ����C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int basic_cnt_ins(FILE *fp)
{
    int ins_line ;             /* �ǉ����C���� */
    int ins_line_tmp ;         /* �P�̃J�E���g�J�n�^�I���R�����g�Ԃ̒ǉ����C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    int str_pos ;              /* �󔒈ȊO�̕����J�n�ʒu */
    int count_start_flg ;      /* �����J�E���g���J�E���g�J�n�����񔭌��t���O */
    int i ;                    /* ���[�v�J�E���^ */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    ins_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            count_start_flg = LINE_COUNT_OFF ;  /* �J�E���g�J�n�����񔭌��t���O�������� */
            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].ins_cnt = 0 ;  /* �����ǉ����C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �ǉ����C�������u0�v�̏ꍇ�A�u1�v�ɂ��� */
            /* �������A�J�E���g�J�n�����񂪕����J�E���g�ӏ��ɂȂ������ꍇ�́u0�v�̂܂� */
            /* ��if���̏����̂ݒǉ��ȂǂɑΉ����邽�� */
            if ( ( part_line_tmp == 0 ) &&
                 ( count_start_flg == LINE_COUNT_ON ) )
            {
                part_line_tmp++ ;
            }

            /* �����ǉ����C������ޔ� */
            sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�J�n�����񔭌��t���O��ON */
            count_start_flg = LINE_COUNT_ON ;

            /* �P�̃J�E���g�J�n�^�I���R�����g�Ԃ̒ǉ����C������������ */
            ins_line_tmp = 0 ;

            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ǉ��̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    count_start_flg = LINE_COUNT_OFF ;  /* �J�E���g�J�n�����񔭌��t���O�������� */
                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].ins_cnt = 0 ;  /* �����ǉ����C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �ǉ����C�������u0�v�̏ꍇ�A�u1�v�ɂ��� */
                    /* ��if���̏����̂ݒǉ��ȂǂɑΉ����邽�� */
                    if ( part_line_tmp == 0 )
                    {
                        part_line_tmp++ ;
                    }

                    /* �����ǉ����C������ޔ� */
                    sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    /* �ǉ����C�������u0�v�̏ꍇ�A�u1�v�ɂ��� */
                    /* ��if���̏����̂ݒǉ��ȂǂɑΉ����邽�� */
                    if ( ins_line_tmp == 0 )
                    {
                        ins_line_tmp++ ;
                    }

                    break ;    /* ���̃J�E���g�J�n��������������� */
                }

                /* �󔒈ȊO�̕�����擪�ʒu���`�F�b�N */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_pos�̃C���N�������g��for���� */
                }

                /* ����������i�R�����g�s�j�Ȃ玟�̍s�� */
                if ( ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '*' ) ) ||
                     ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '/' ) ) )
                {
                    continue ;
                }


                /* ���C�����J�E���g���Ɍ������镶����e�[�u���I�[�܂Ń��[�v */
                for ( i = 0; strcmp(cLine_count_str_tbl[i], "\0") != 0  ; i++ )
                {
                    /* ���C�����J�E���g������e�[�u�����u�J���}�v�łȂ��ꍇ */
                    if ( i != 0 )
                    {
                         /* ���C���������ƂȂ镶����𔭌� */
                        if ( strstr(buf, cLine_count_str_tbl[i]) != NULL )
                        {
                            ins_line_tmp++ ;    /* �ǉ����C�������C���N�������g */
                            part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                            break ;
                        }
                    }
                    else
                    {
                        /* ���C���������ƂȂ镶����(�J���})�𔭌� ���� �J���}�J�E���gON�̏ꍇ*/
                        if ( ( strstr(buf, cLine_count_str_tbl[i]) != NULL ) &&
                             ( sSearch_info.comma_cnt_flg == LINE_COUNT_ON ) )
                        {
                            ins_line_tmp++ ;    /* �ǉ����C�������C���N�������g */
                            part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                            break ;
                        }
                    }
                }
            }

            /* �ǉ����C�����Ɂu�P�̃J�E���g�J�n�^�I���R�����g�Ԃ̒ǉ����C�����v�����Z */
            ins_line = ins_line + ins_line_tmp ;
        }
    }

    return ins_line ;    /* �ǉ����C������Ԃ� */
}

/*******************************************************************************
* MODULE        : basic_cnt_chg
* ABSTRACT      : ��{�J�E���g �ύX���C�����J�E���g�֐�
* FUNCTION      : ��{�J�E���g �ύX���C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�ύX���C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int basic_cnt_chg(FILE *fp)
{
    int chg_line ;             /* �ύX���C���� */
    int chg_line_tmp ;         /* �P�̃J�E���g�J�n�^�I���R�����g�Ԃ̕ύX���C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    int str_pos ;              /* �󔒈ȊO�̕����J�n�ʒu */
    int count_start_flg ;      /* �����J�E���g���J�E���g�J�n�����񔭌��t���O */
    int i ;                    /* ���[�v�J�E���^ */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    chg_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            count_start_flg = LINE_COUNT_OFF ;  /* �J�E���g�J�n�����񔭌��t���O�������� */
            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].chg_cnt = 0 ;  /* �����ύX���C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �ύX���C�������u0�v�̏ꍇ�A�u1�v�ɂ��� */
            /* ��if���̏����̂ݒǉ��ȂǂɑΉ����邽�� */
            if ( ( part_line_tmp == 0 ) &&
                 ( count_start_flg == LINE_COUNT_ON ) )
            {
                part_line_tmp++ ;
            }

            /* �����ύX���C������ޔ� */
            sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�J�n�����񔭌��t���O��ON */
            count_start_flg = LINE_COUNT_ON ;

            /* �P�̃J�E���g�J�n�^�I���R�����g�Ԃ̕ύX���C������������ */
            chg_line_tmp = 0 ;

            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ύX�̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    count_start_flg = LINE_COUNT_OFF ;  /* �J�E���g�J�n�����񔭌��t���O�������� */
                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].chg_cnt = 0 ;  /* �����ύX���C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �ǉ����C�������u0�v�̏ꍇ�A�u1�v�ɂ��� */
                    /* ��if���̏����̂ݒǉ��ȂǂɑΉ����邽�� */
                    if ( part_line_tmp == 0 )
                    {
                        part_line_tmp++ ;
                    }

                    /* �����ǉ����C������ޔ� */
                    sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    /* �ύX���C�������u0�v�̏ꍇ�A�u1�v�ɂ��� */
                    /* ��if���̏����̂ݕύX�ȂǂɑΉ����邽�� */
                    if ( chg_line_tmp == 0 )
                    {
                        chg_line_tmp++ ;
                    }

                    break ;    /* ���̃J�E���g�J�n��������������� */
                }

                /* �󔒈ȊO�̕�����擪�ʒu���`�F�b�N */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_pos�̃C���N�������g��for���� */
                }

                /* ����������i�R�����g�s�jand �u�ύX�v�ɂ��R�����g�A�E�g�����J�E���g���Ȃ� */
                /* �Ȃ�΁A���̍s�� */
                if ( ( ( ( buf[str_pos] == '/' ) &&
                         ( buf[str_pos + 1 ] == '*' ) ) ||
                       ( ( buf[str_pos] == '/' ) &&
                         ( buf[str_pos + 1 ] == '/' ) ) ) &&
                     ( sSearch_info.chg_com_cnt_flg == LINE_COUNT_OFF ) )
                {
                    continue ;
                }

                /* ���C�����J�E���g���Ɍ������镶����e�[�u���I�[�܂Ń��[�v */
                for ( i = 0; strcmp(cLine_count_str_tbl[i], "\0") != 0  ; i++ )
                {
                    /* ���C�����J�E���g������e�[�u�����u�J���}�v�łȂ��ꍇ */
                    if ( i != 0 )
                    {
                         /* ���C���������ƂȂ镶����𔭌� */
                        if ( strstr(buf, cLine_count_str_tbl[i]) != NULL )
                        {
                            chg_line_tmp++ ;    /* �ύX���C�������C���N�������g */
                            part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                            break ;
                        }
                    }
                    else
                    {
                        /* ���C���������ƂȂ镶����(�J���})�𔭌� ���� �J���}�J�E���gON�̏ꍇ*/
                        if ( ( strstr(buf, cLine_count_str_tbl[i]) != NULL ) &&
                             ( sSearch_info.comma_cnt_flg == LINE_COUNT_ON ) )
                        {
                            chg_line_tmp++ ;    /* �ύX���C�������C���N�������g */
                            part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                            break ;
                        }
                    }
                }
            }

            /* �ύX���C�����Ɂu�P�̃J�E���g�J�n�^�I���R�����g�Ԃ̕ύX���C�����v�����Z */
            chg_line = chg_line + chg_line_tmp ;
        }
    }

    return chg_line ;    /* �ύX���C������Ԃ� */
}

/*******************************************************************************
* MODULE        : basic_cnt_del
* ABSTRACT      : ��{�J�E���g �폜���C�����J�E���g�֐�
* FUNCTION      : ��{�J�E���g �폜���C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�폜���C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int basic_cnt_del(FILE *fp)
{
    int del_line ;             /* �폜���C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    int i ;                    /* ���[�v�J�E���^ */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    del_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].del_cnt = 0 ;  /* �����폜���C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����폜���C������ޔ� */
            sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�폜�̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* ���������J�n�����񔭌� */
                if ( strstr(buf, INVALID_START) != NULL )
                {
                    /* ���������I���������������܂Ń��[�v */
                    while(1)
                    {
                        /* ������1�s�ǂݍ��� */
                        ret_p = fgets(buf,READ_SIZE,fp) ;

                        /* ���������I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                        if ( ret_p == NULL )
                        {
                            printf("\n----------------------------------------------------------------------\n") ;
                            printf("���������J�n�^�I���̃R�����g���s���ł�\n") ;
                            return LINE_COUNT_NG ;
                        }

                        /* ���������I�������񔭌� */
                        if ( strstr(buf, INVALID_END) != NULL )
                        {
                            break ;    /* ���[�v�𔲂��� */
                        }
                    }
                    continue ;    /* ���̍s��ǂݍ��� */
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].del_cnt = 0 ;  /* �����폜���C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����폜���C������ޔ� */
                    sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }

                /* ���C�����J�E���g���Ɍ������镶����e�[�u���I�[�܂Ń��[�v */
                for ( i = 0; strcmp(cLine_count_str_tbl[i], "\0") != 0  ; i++ )
                {
                    /* ���C�����J�E���g������e�[�u�����u�J���}�v�łȂ��ꍇ */
                    if ( i != 0 )
                    {
                         /* ���C���������ƂȂ镶����𔭌� */
                        if ( strstr(buf, cLine_count_str_tbl[i]) != NULL )
                        {
                            del_line++ ;        /* �폜���C�������C���N�������g */
                            part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                            break ;
                        }
                    }
                    else
                    {
                        /* ���C���������ƂȂ镶����(�J���})�𔭌� ���� �J���}�J�E���gON�̏ꍇ*/
                        if ( ( strstr(buf, cLine_count_str_tbl[i]) != NULL ) &&
                             ( sSearch_info.comma_cnt_flg == LINE_COUNT_ON ) )
                        {
                            del_line++ ;        /* �폜���C�������C���N�������g */
                            part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                            break ;
                        }
                    }
                }
            }
        }
    }

    return del_line ;    /* �폜���C������Ԃ� */
}

/*******************************************************************************
* MODULE        : line_cnt_ins
* ABSTRACT      : ���C�������̂܂܃J�E���g �ǉ����C�����J�E���g�֐�
* FUNCTION      : ���C�������̂܂܃J�E���g �ǉ����C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�ǉ����C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int line_cnt_ins(FILE *fp)
{
    int ins_line ;             /* �ǉ����C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    int str_pos ;              /* �󔒈ȊO�̕����J�n�ʒu */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    ins_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].ins_cnt = 0 ;  /* �����ǉ����C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����ǉ����C������ޔ� */
            sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ǉ��̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].ins_cnt = 0 ;  /* �����ǉ����C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����ǉ����C������ޔ� */
                    sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }

                /* �󔒈ȊO�̕�����擪�ʒu���`�F�b�N */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_pos�̃C���N�������g��for���� */
                }

                /* ����������i�R�����g�s�j or �󔒂݂̂̍s�Ȃ玟�̍s�� */
                if ( ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '*' ) ) ||
                     ( ( buf[str_pos] == '/' ) &&
                       ( buf[str_pos + 1 ] == '/' ) ) ||
                     ( buf[str_pos] == '\n' ) )
                {
                    continue ;
                }

                /* ���s�ȊO�͒ǉ����C�������C���N�������g */
                if ( buf[0] != '\n' )
                {
                    ins_line++ ;        /* �ǉ����C�������C���N�������g */
                    part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                }
            }
        }
    }

    return ins_line ;    /* �ǉ����C������Ԃ� */
}

/*******************************************************************************
* MODULE        : line_cnt_chg
* ABSTRACT      : ���C�������̂܂܃J�E���g �ύX���C�����J�E���g�֐�
* FUNCTION      : ���C�������̂܂܃J�E���g �ύX���C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�ύX���C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int line_cnt_chg(FILE *fp)
{
    int chg_line ;             /* �ύX���C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    int str_pos ;              /* �󔒈ȊO�̕����J�n�ʒu */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    chg_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].chg_cnt = 0 ;  /* �����ύX���C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����ύX���C������ޔ� */
            sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ύX�̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].chg_cnt = 0 ;  /* �����ύX���C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����ǉ����C������ޔ� */
                    sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }

                /* �󔒈ȊO�̕�����擪�ʒu���`�F�b�N */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_pos�̃C���N�������g��for���� */
                }

                /* (����������i�R�����g�s�jand �u�ύX�v�ɂ��R�����g�A�E�g�����J�E���g���Ȃ�) */
                /*  or �󔒂݂̂̍s�Ȃ玟�̍s�� */
                if ( ( ( ( ( buf[str_pos] == '/' ) &&
                           ( buf[str_pos + 1 ] == '*' ) ) ||
                         ( ( buf[str_pos] == '/' ) &&
                           ( buf[str_pos + 1 ] == '/' ) ) ) &&
                         ( sSearch_info.chg_com_cnt_flg == LINE_COUNT_OFF ) ) ||
                     ( buf[str_pos] == '\n' ) )
                {
                    continue ;
                }

                /* ���s�ȊO�͕ύX���C�������C���N�������g */
                if ( buf[0] != '\n' )
                {
                    chg_line++ ;        /* �ύX���C�������C���N�������g */
                    part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                }
            }
        }
    }

    return chg_line ;    /* �ύX���C������Ԃ� */
}

/*******************************************************************************
* MODULE        : line_cnt_del
* ABSTRACT      : ���C�������̂܂܃J�E���g �폜���C�����J�E���g�֐�
* FUNCTION      : ���C�������̂܂܃J�E���g �폜���C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�폜���C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int line_cnt_del(FILE *fp)
{
    int del_line ;             /* �폜���C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    int str_pos ;              /* �󔒈ȊO�̕����J�n�ʒu */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    del_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].del_cnt = 0 ;  /* �����폜���C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����폜���C������ޔ� */
            sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�폜�̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* ���������J�n�����񔭌� */
                if ( strstr(buf, INVALID_START) != NULL )
                {
                    /* ���������I���������������܂Ń��[�v */
                    while(1)
                    {
                        /* ������1�s�ǂݍ��� */
                        ret_p = fgets(buf,READ_SIZE,fp) ;

                        /* ���������I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                        if ( ret_p == NULL )
                        {
                            printf("\n----------------------------------------------------------------------\n") ;
                            printf("���������J�n�^�I���̃R�����g���s���ł�\n") ;
                            return LINE_COUNT_NG ;
                        }

                        /* ���������I�������񔭌� */
                        if ( strstr(buf, INVALID_END) != NULL )
                        {
                            break ;    /* ���[�v�𔲂��� */
                        }
                    }
                    continue ;    /* ���̍s��ǂݍ��� */
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].del_cnt = 0 ;  /* �����폜���C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����폜���C������ޔ� */
                    sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }

                /* �󔒈ȊO�̕�����擪�ʒu���`�F�b�N */
                for ( str_pos = 0; buf[str_pos] == ' ' ; str_pos++ )
                {
                    /* str_pos�̃C���N�������g��for���� */
                }

                /* �󔒂݂̂̍s�Ȃ玟�̍s�� */
                if ( buf[str_pos] == '\n' )
                {
                    continue ;
                }

                /* ���s�ȊO�͍폜���C�������C���N�������g */
                if ( buf[0] != '\n' )
                {
                    del_line++ ;        /* �폜���C�������C���N�������g */
                    part_line_tmp++ ;   /* �����J�E���g���C�������C���N�������g */
                }
            }
        }
    }

    return del_line ;    /* �폜���C������Ԃ� */
}

/*******************************************************************************
* MODULE        : increase_cnt_ins
* ABSTRACT      : �Ђ�����J�E���g �ǉ����C�����J�E���g�֐�
* FUNCTION      : �Ђ�����J�E���g �ǉ����C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�ǉ����C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int increase_cnt_ins(FILE *fp)
{
    int ins_line ;             /* �ǉ����C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    ins_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].ins_cnt = 0 ;  /* �����ǉ����C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����ǉ����C������ޔ� */
            sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ǉ��̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].ins_cnt = 0 ;  /* �����ǉ����C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����ǉ����C������ޔ� */
                    sPart_count[sPart_count_no].ins_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.ins_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }
                /* 1�s�ǂݍ��ݖ��ɒǉ����C�����A�����J�E���g���C�������C���N�������g */
                else
                {
                    ins_line++ ;
                    part_line_tmp++ ;
                }
            }
        }
    }

    return ins_line ;    /* �ǉ����C������Ԃ� */
}

/*******************************************************************************
* MODULE        : increase_cnt_chg
* ABSTRACT      : �Ђ�����J�E���g �ύX���C�����J�E���g�֐�
* FUNCTION      : �Ђ�����J�E���g �ύX���C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�ύX���C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int increase_cnt_chg(FILE *fp)
{
    int chg_line ;             /* �ύX���C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    chg_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].chg_cnt = 0 ;  /* �����ύX���C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����ύX���C������ޔ� */
            sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�ύX�̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].chg_cnt = 0 ;  /* �����ύX���C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����ǉ����C������ޔ� */
                    sPart_count[sPart_count_no].chg_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.chg_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }
                /* 1�s�ǂݍ��ݖ��ɕύX���C�����A�����J�E���g���C�������C���N�������g */
                else
                {
                    chg_line++ ;
                    part_line_tmp++ ;
                }
            }
        }
    }

    return chg_line ;    /* �ύX���C������Ԃ� */
}

/*******************************************************************************
* MODULE        : increase_cnt_del
* ABSTRACT      : �Ђ�����J�E���g �폜���C�����J�E���g�֐�
* FUNCTION      : �Ђ�����J�E���g �폜���C�����J�E���g
* NOTE          : ��1�����F�ǂݍ��ރt�@�C���|�C���^(IN)
* RETURN        : ����I���F�폜���C����
*                 �ُ�I���FLINE_COUNT_NG
*******************************************************************************/
int increase_cnt_del(FILE *fp)
{
    int del_line ;             /* �폜���C���� */
    int part_line_tmp ;        /* �����J�E���g���C���� */
    char buf[READ_SIZE + 1] ;  /* �t�@�C���ǂݍ��݃o�b�t�@ */
    char *ret_p ;              /* fgets�̖߂�l */

    /* ������ */
    del_line = 0 ;
    memset(buf, '\0', sizeof(buf)) ;

    /* �����J�E���g���s�񐔏����� */
    sPart_count_no = 0 ;

    /* �t�@�C���I�[�܂ŕ������1�s���ǂݍ��� */
    for( ; (fgets(buf,READ_SIZE,fp)) != NULL ; )
    {
        /* �����J�E���g�J�n�����񔭌� */
        if ( strstr(buf, PART_COUNT_START) != NULL )
        {
            /* �����J�E���g�\���I�[�o�[ */
            if ( sPart_count_no > PART_COUNT_MAX )
            {
                printf("\n----------------------------------------------------------------------\n") ;
                printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                return LINE_COUNT_NG ;
            }

            part_line_tmp = 0 ;
            sPart_count[sPart_count_no].del_cnt = 0 ;  /* �����폜���C������������ */
        }
        /* �����J�E���g�I�������񔭌� */
        else if ( strstr(buf, PART_COUNT_END) != NULL )
        {
            /* �����폜���C������ޔ� */
            sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
            sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
        }

        /* �J�E���g�J�n�����񔭌� */
        if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
             ( strstr(buf, sSearch_info.key_word) != NULL ) &&
             ( strstr(buf, sSearch_info.start_key) != NULL ) )
        {
            /* �J�E���g�I���������������܂Ń��[�v */
            while(1)
            {
                /* ������1�s�ǂݍ��� */
                ret_p = fgets(buf,READ_SIZE,fp) ;

                /* �J�E���g�I�������񔭌��O�Ƀt�@�C���ǂݍ��݂��I�� */
                if ( ret_p == NULL )
                {
                    printf("\n----------------------------------------------------------------------\n") ;
                    printf("�폜�̃J�E���g�J�n�^�I���̃R�����g���s���ł�\n") ;
                    return LINE_COUNT_NG ;
                }

                /* �����J�E���g�J�n�����񔭌� */
                if ( strstr(buf, PART_COUNT_START) != NULL )
                {
                    /* �����J�E���g�\���I�[�o�[ */
                    if ( sPart_count_no > PART_COUNT_MAX )
                    {
                        printf("\n----------------------------------------------------------------------\n") ;
                        printf("�����J�E���g�\�����I�[�o�[���Ă��܂�\n") ;
                        return LINE_COUNT_NG ;
                    }

                    part_line_tmp = 0 ;
                    sPart_count[sPart_count_no].del_cnt = 0 ;  /* �����폜���C������������ */
                }
                /* �����J�E���g�I�������񔭌� */
                else if ( strstr(buf, PART_COUNT_END) != NULL )
                {
                    /* �����폜���C������ޔ� */
                    sPart_count[sPart_count_no].del_cnt = part_line_tmp ;
                    sPart_count_no++ ;    /* ���̕����J�E���g���J�n */
                }

                /* �J�E���g�I�������񔭌� */
                if ( ( strstr(buf, sSearch_info.del_key) != NULL ) &&
                     ( strstr(buf, sSearch_info.key_word) != NULL ) &&
                     ( strstr(buf, sSearch_info.end_key) != NULL ) )
                {
                    break ;    /* ���̃J�E���g�J�n��������������� */
                }
                /* 1�s�ǂݍ��ݖ��ɍ폜���C�����A�����J�E���g���C�������C���N�������g */
                else
                {
                    del_line++ ;
                    part_line_tmp++ ;
                }
            }
        }
    }

    return del_line ;    /* �폜���C������Ԃ� */
}

/*******************************************************************************
* MODULE        : cnt_explain
* ABSTRACT      : �J�E���g���@�����֐�
* FUNCTION      : �J�E���g���@���������邾��
* NOTE          : 
* RETURN        : �Ȃ�
*******************************************************************************/
void cnt_explain(void)
{
    printf("\n----------------------------------------------------------------------\n") ;
    puts("����{�J�E���g") ;
    puts("  �E�ȉ��̕�������s���ɔ�������ƁA1���C���Ɛ�����") ;
    puts("    �ˁu;�v�uif�v�uelse if�v�uelse�v�uswitch�v�ucase�v�udefault�v") ;
    puts("      �ufor�v�uwhile�v�u#define�v") ;
    puts("  �E�������͏�������̃��C�����ɂ�炸1���C���ƂȂ�") ;
    puts("  ���u,�v�̃��C�����J�E���g�͍ŏ��ɑI�������J�E���g���@�ɂ��") ;

    puts("\n�����C�������̂܂܃J�E���g") ;
    puts("  �E�u�󔒂̂݁vor�u���s�̂݁v�ȊO�̃��C������1���C���Ɛ�����") ;
    puts("  �E�������͏�������̃��C���������J�E���g����") ;
    puts("  �E�u{�v�u}�v�݂̂̍s��1���C���ɃJ�E���g�����") ;

    puts("\n���Ђ�����J�E���g") ;
    puts("  �E�Ƃ肠���������Ȃ�悤�ɐ����Ă݂�") ;
}

