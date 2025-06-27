#ifndef JIMP_H_
#define JIMP_H_

#include "stb_c_lexer.h"

typedef struct {
    stb_lexer l;
    const char *path;
    const char *key;
} Jimp;

bool jimp_bool(Jimp *jimp, bool *boolean);
bool jimp_number(Jimp *jimp, long *number);
bool jimp_string(Jimp *jimp, const char **string);
bool jimp_object_begin(Jimp *jimp);
bool jimp_object_member(Jimp *jimp);
bool jimp_object_end(Jimp *jimp);
void jimp_unknown_member(Jimp *jimp);
bool jimp_array_begin(Jimp *jimp);
bool jimp_array_end(Jimp *jimp);
bool jimp_array_has_items(Jimp *jimp);

// TODO: should be private
bool jimp_expect_token(Jimp *jimp, long token);
bool jimp_get_and_expect_token(Jimp *jimp, long token);
const char *jimp_token_kind(long token);

#endif // JIMP_H_

#ifdef JIMP_IMPLEMENTATION

const char *jimp_token_kind(long token)
{
   switch (token) {
      case CLEX_id        : return("identifier");
      case CLEX_eq        : return("==");
      case CLEX_noteq     : return("!=");
      case CLEX_lesseq    : return("<=");
      case CLEX_greatereq : return(">=");
      case CLEX_andand    : return("&&");
      case CLEX_oror      : return("||");
      case CLEX_shl       : return("<<");
      case CLEX_shr       : return(">>");
      case CLEX_plusplus  : return("++");
      case CLEX_minusminus: return("--");
      case CLEX_arrow     : return("->");
      case CLEX_andeq     : return("&=");
      case CLEX_oreq      : return("|=");
      case CLEX_xoreq     : return("^=");
      case CLEX_pluseq    : return("+=");
      case CLEX_minuseq   : return("-=");
      case CLEX_muleq     : return("*=");
      case CLEX_diveq     : return("/=");
      case CLEX_modeq     : return("%%=");
      case CLEX_shleq     : return("<<=");
      case CLEX_shreq     : return(">>=");
      case CLEX_eqarrow   : return("=>");
      case CLEX_dqstring  : return("string");
      case CLEX_sqstring  : return("single quote string");
      case CLEX_charlit   : return("character");
      case CLEX_intlit    : return("integer");
      case CLEX_floatlit  : return("float");
      default:
         if (token >= 0 && token < 256)
            return temp_sprintf("%c", (int) token);
         else {
            return temp_sprintf("<<<UNKNOWN TOKEN %ld >>>\n", token);
         }
         break;
   }
}

bool jimp_array_begin(Jimp *jimp)
{
    return jimp_get_and_expect_token(jimp, '[');
}

bool jimp_array_end(Jimp *jimp)
{
    return jimp_get_and_expect_token(jimp, ']');
}

bool jimp_array_has_items(Jimp *jimp)
{
    char *point = jimp->l.parse_point;
    if (!stb_c_lexer_get_token(&jimp->l)) return false;
    if (jimp->l.token == ',') return true;
    if (jimp->l.token == ']') {
        jimp->l.parse_point = point;
        return false;
    }
    jimp->l.parse_point = point;
    return true;
}

void jimp_unknown_member(Jimp *jimp)
{
    stb_lex_location loc = {0};
    stb_c_lexer_get_location(&jimp->l, jimp->l.where_firstchar, &loc);
    fprintf(stderr, "%s:%d:%d: ERROR: Unexpected member `%s`\n", jimp->path, loc.line_number, loc.line_offset + 1, jimp->key);
}

bool jimp_object_begin(Jimp *jimp)
{
    return jimp_get_and_expect_token(jimp, '{');
}

bool jimp_object_member(Jimp *jimp)
{
    char *point = jimp->l.parse_point;
    if (!stb_c_lexer_get_token(&jimp->l)) return false;
    if (jimp->l.token == ',') {
        if (!jimp_get_and_expect_token(jimp, CLEX_dqstring)) return false;
        jimp->key = strdup(jimp->l.string); // TODO: memory leak
        if (!jimp_get_and_expect_token(jimp, ':')) return false;
        return true;
    }
    if (jimp->l.token == '}') {
        jimp->l.parse_point = point;
        return false;
    }
    if (!jimp_expect_token(jimp, CLEX_dqstring)) return false;
    jimp->key = strdup(jimp->l.string); // TODO: memory leak
    if (!jimp_get_and_expect_token(jimp, ':')) return false;
    return true;
}

bool jimp_object_end(Jimp *jimp)
{
    return jimp_get_and_expect_token(jimp, '}');
}

bool jimp_string(Jimp *jimp, const char **string)
{
    if (!jimp_get_and_expect_token(jimp, CLEX_dqstring)) return false;
    *string = strdup(jimp->l.string);
    return true;
}

bool jimp_bool(Jimp *jimp, bool *boolean)
{
    if (!jimp_get_and_expect_token(jimp, CLEX_id)) return false;
    if (strcmp(jimp->l.string, "true") == 0) {
        *boolean = true;
    } else if (strcmp(jimp->l.string, "false") == 0) {
        *boolean = false;
    } else {
        stb_lex_location loc = {0};
        stb_c_lexer_get_location(&jimp->l, jimp->l.where_firstchar, &loc);
        fprintf(stderr, "%s:%d:%d: ERROR: Expected boolean but got `%s`\n", jimp->path, loc.line_number, loc.line_offset + 1, jimp_token_kind(jimp->l.token));
        return false;
    }
    return true;
}

bool jimp_number(Jimp *jimp, long *number)
{
    // TODO: there are more things that constitude number in JSON, for example floats. Take all of them into account here.
    if (!jimp_get_and_expect_token(jimp, CLEX_intlit)) return false;
    *number = jimp->l.int_number;
    return true;
}

bool jimp_get_and_expect_token(Jimp *jimp, long token)
{
    if (!stb_c_lexer_get_token(&jimp->l)) return false;
    return jimp_expect_token(jimp, token);
}

bool jimp_expect_token(Jimp *jimp, long token)
{
    if (jimp->l.token != token) {
        stb_lex_location loc = {0};
        stb_c_lexer_get_location(&jimp->l, jimp->l.where_firstchar, &loc);
        fprintf(stderr, "%s:%d:%d: ERROR: expected %s, but got %s\n", jimp->path, loc.line_number, loc.line_offset + 1, jimp_token_kind(token), jimp_token_kind(jimp->l.token));
        return false;
    }
    return true;
}

#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#endif // JIMP_IMPLEMENTATION
