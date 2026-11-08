#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <threads.h>

#define ASSERT(condition)                \
do {                                     \
   if(!(condition))                      \
   {                                     \
      fprintf                            \
      (                                  \
      stderr,                            \
      "[ASSERTION FAIL] "                \
      "content: %s | "                   \
      "file: %s | "                      \
      "line: %d | "                      \
      "\n",                              \
                                         \
      #condition, __FILE__, __LINE__     \
      );                                 \
      abort();                           \
   }                                     \
} while(0)                               \


#define TOKENS \
   X(NoneYet)          \
   X(TypeDeclaration)  \
   X(Identifier)       \
   X(Assignment)       \
   X(IntegerLiteral)   \
   X(FloatLiteral)     \
   X(StringLiteral)    \
   X(BooleanLiteral)   \
   X(StatementEnd)    

/* part vector */
struct vector
{
  size_t element_count;
  size_t capacity;
  size_t element_size;
  void*  data_ptr;
};

struct vector*
vector_create(size_t single_element_size)
{
   struct vector* vector     = malloc(sizeof(struct vector));
   ASSERT(NULL != vector);

   size_t initial_vector_capacity = 4;
                                  /*      NOTE: overflow could happen here      */
   size_t initial_vector_size     = initial_vector_capacity * single_element_size;
   void*  data_ptr                = malloc(initial_vector_size);
   ASSERT(NULL != data_ptr);

   vector->element_count        = 0;
   vector->capacity             = initial_vector_capacity;
   vector->element_size         = single_element_size;
   vector->data_ptr             = data_ptr;

   return vector;
}

void
vector_append(struct vector* self, void* element_ptr)
{
   if(self->element_count == self->capacity)
   {
      size_t new_capacity  = self->capacity * 2;
      size_t new_byte_size = new_capacity   * self->element_size;

      void* new_data_ptr = realloc(self->data_ptr, new_byte_size);
      ASSERT(NULL != new_data_ptr);
      self->data_ptr = new_data_ptr;
      self->capacity = new_capacity;
   }

   void *first_empty_space_ptr =
       (char*)self->data_ptr + (self->element_size * self->element_count);

   memcpy(first_empty_space_ptr, element_ptr, self->element_size);
   self->element_count = self->element_count + 1;
}

void*
vector_get(struct vector* self, size_t index)
{
   ASSERT (index < self->element_count);
   return ((char*)self->data_ptr) + (self->element_size * index);
}

void
vector_free(struct vector* self)
{
   free(self->data_ptr);
   free(self);
}

/* part lexer */
enum TokenKind
{
   #define X(name) name,
   TOKENS
   #undef X
};

char*
token_kind_to_str(enum TokenKind kind)
{
   #define X(name) if(kind == name) { static char* variable = #name; return variable; }
   TOKENS
   #undef X

   ASSERT("no execution path leads here" && 0);
}

struct Token
{
   enum TokenKind kind;
   char* value_ptr;
   size_t len;
};

bool
is_alpha(char character)
{
   bool is_uppercase_ascii = character >= 65 && character <= 90;
   bool is_lowercase_ascii = character >= 97 && character <= 122;

   if(is_uppercase_ascii || is_lowercase_ascii) {
      return true;
   }

   return false;
}

bool
is_numeric(char character)
{
   return character >= 48 && character <= 57;
}

struct Lexer
{
   struct vector* tokens;

   size_t current_index;
   char current_character;
   char* characters;
   size_t character_count;
   bool end_of_characters;

   // NOTE: for error location: line X, column Y
   size_t encountered_newlines;
   size_t last_newline_index;
};

struct Lexer*
lexer_create(char* characters, size_t character_count)
{
   struct Lexer* lexer          = malloc(sizeof(struct Lexer));
   struct vector* tokens_vector = vector_create(sizeof(struct Token));
   ASSERT(lexer != NULL );

   lexer->characters        = characters;
   lexer->tokens            = tokens_vector;
   lexer->character_count   = character_count;
   lexer->end_of_characters = false;

   lexer->current_index     = 0;
   lexer->current_character = lexer->characters[0];

   lexer->encountered_newlines = 0;
   lexer->last_newline_index   = 0;

   return lexer;
}

/* does not deallocate data under received pointer to characters */
void
lexer_free(struct Lexer* self)
{
   free(self->tokens);
   free(self);
}

void
lexer_advance(struct Lexer* self)
{
   if(self->current_character == '\n')
   {
      self->encountered_newlines++;
      self->last_newline_index = self->current_index;
   }

   if(self->current_index + 1 < self->character_count)
   {
      self->current_index++;
      self->current_character = self->characters[self->current_index];
      return;
   };

   self->end_of_characters = true;
   return;

}

char*
lexer_peek(struct Lexer* self)
{
   if(self->current_index + 1 < self->character_count)
   {
      return &self->characters[self->current_index + 1];
   }

   return NULL;
}

void
lexer_identifier(struct Lexer* self)
{
   char*  start_ptr            = self->characters + self->current_index;
   size_t lexeme_length        = 1; // single char initially
   lexer_advance(self);

   while(!self->end_of_characters && is_alpha(self->current_character))
   {
      ++lexeme_length; 
      lexer_advance(self);
   }

   struct Token identifier_token =
   {
     .value_ptr = start_ptr,
     .len       = lexeme_length,
     .kind      = Identifier
   };

   vector_append(self->tokens, &identifier_token);
}

void
lexer_number(struct Lexer* self)
{
   char*  start_ptr            = self->characters + self->current_index;
   size_t lexeme_length        = 1; // single char initially

   lexer_advance(self);
   while(!self->end_of_characters && is_numeric(self->current_character))
   {
      ++lexeme_length; 
      lexer_advance(self);
   }

   struct Token identifier_token =
   {
     .value_ptr = start_ptr,
     .len       = lexeme_length,
     .kind      = IntegerLiteral
   };

   vector_append(self->tokens, &identifier_token);
}

#define LEX_RESULT_CODE  \
   X(AllGood)            \
   X(InvalidCharacter)

enum LexResultCode
{
   #define X(name) name,
   LEX_RESULT_CODE
   #undef X
};

struct LexResult
{
   enum LexResultCode code;
   // when result != AllGood
   char* error_message;
};
   

char*
lex_result_to_str(enum LexResultCode code)
{
   #define X(name) if(code == name) { static char* variable = #name; return variable; }
   LEX_RESULT_CODE
   #undef X

   ASSERT("no execution path leads here" && false);
}
   
struct LexResult
lexer_lex(struct Lexer* self)
{
   char magic_compiler_blocker = '&';
   
   while(!(self->end_of_characters))
   {
      if(self->current_character == magic_compiler_blocker)
      {
         break;
      }

      if(self->current_character == ' ' || self->current_character == '\n')
      {
         lexer_advance(self);
         continue;
      }

      if(is_alpha(self->current_character))
      {
         lexer_identifier(self);
         continue;
      }

      if(self->current_character == '=')
      {
         struct Token identifier_token =
         {
           .value_ptr = self->characters + self->current_index,
           .len       = 1,
           .kind      = Assignment
         };
         vector_append(self->tokens, &identifier_token);
         lexer_advance(self);
         continue;
      }

      if(self->current_character == ';')
      {
         struct Token identifier_token =
         {
           .value_ptr = self->characters + self->current_index,
           .len       = 1,
           .kind      = StatementEnd
         };
         vector_append(self->tokens, &identifier_token);
         lexer_advance(self);
         continue;
      }      

      if(is_numeric(self->current_character))
      {
         lexer_number(self);
         continue;
      }

      size_t account_for_being_on_index_0 = self->current_index == 0 ? 1 : 0;
      // make both 1-indexed
      size_t column = self->current_index - self->last_newline_index + account_for_being_on_index_0; 
      size_t line   = self->encountered_newlines + 1;

      static char error_message[256];
      snprintf
      (
         error_message, sizeof(error_message),
         "invalid character '%c' encountered on line %zu, column %zu",
         self->current_character,line, column
      );

      return
      (struct LexResult)
      {
      .code          = InvalidCharacter,
      .error_message = error_message
      };
   }

   return
   (struct LexResult)
   {
   .code          = AllGood,
   .error_message = NULL
   };
}


/* part main */
int
main(void)
{
   // read the file
   FILE* compiled_file = fopen("./delight-editor/start", "r");

   if(NULL == compiled_file)
   {
      printf("file opening has failed");
      return -1;
   }

   char file_content_buffer[1024];

   size_t fread_file_byte_count = fread(file_content_buffer, sizeof(char), sizeof(file_content_buffer), compiled_file);
   if(0 != ferror(compiled_file))
   {
      printf("loading file into memory has failed");
      return -1;
   }

   // (f)lex
   struct Lexer* lexer = lexer_create(file_content_buffer, fread_file_byte_count);
   struct LexResult result = lexer_lex(lexer);

   if(result.code != AllGood)
   {
      printf("[LEXER ERROR][%s]: %s\n", lex_result_to_str(result.code), result.error_message);
      return -1;
   }

   for(size_t i = 0; i < lexer->tokens->element_count; i++)
   {
      struct Token *token = vector_get(lexer->tokens, i);
      
      printf
      (
       "kind '%s' | value: '%.*s'\n",
       token_kind_to_str(token->kind),
       (int)token->len, (char*)token->value_ptr
      );
   }

   lexer_free(lexer);

   return 0;
}
