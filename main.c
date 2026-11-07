#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

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
   X(BooleanLiteral)   

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

/* part main */
enum TokenKind
{
   #define X(name) name,
   TOKENS
   #undef X
};

char* token_kind_to_char(enum TokenKind kind) {
   #define X(name) if(kind == name) { static char* variable = #name; return variable; }
   TOKENS
   #undef X

   ASSERT("no execution path leads here" && 0);
}

struct Token
{
   struct vector* value;
   enum TokenKind kind;
};

bool is_alpha(char character) {
   bool is_uppercase_ascii = character >= 65 && character <= 90;
   bool is_lowercase_ascii = character >= 97 && character <= 122;

   if(is_uppercase_ascii || is_lowercase_ascii) {
      return true;
   }

   return false;
}

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
   struct vector* current_lexeme = vector_create(sizeof(char));
   struct vector* tokens = vector_create(sizeof(struct Token));
   
   enum TokenKind assumed_token_kind = NoneYet;

   char magic_compiler_blocker = '&';

   for(size_t i = 0; i < fread_file_byte_count; i++)
   {
      char character = file_content_buffer[i];

      if(character == magic_compiler_blocker) { break; }
      

      if(is_alpha(character))
      {
         assumed_token_kind = Identifier;
         vector_append(current_lexeme, &character);
         continue;
      }


      if((' ' == character || '\n' == character))
      {
         if(current_lexeme->element_count > 0)
         {
            struct Token established_token =
            {
               .kind  = assumed_token_kind,
               .value = current_lexeme
            };

            vector_append(tokens, &established_token);
            current_lexeme = vector_create(sizeof(char));           
         }

         continue;
      }

      printf("file contains invalid characters: '%c'", character);
      return -1;
   }

   printf("tokens found: \n-----\n");
   for(size_t i = 0; i < tokens->element_count; i++)
   {
      struct Token *token = vector_get(tokens, i);
      
      printf
      (
       "kind '%s' | value: '%.*s'\n",
       token_kind_to_char(token->kind),
       (int)token->value->element_count, (char*)token->value->data_ptr
      );
   }
   

   vector_free(current_lexeme);
   vector_free(tokens);

   return 0;
}
