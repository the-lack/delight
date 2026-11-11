#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <threads.h>
#include <unistd.h>

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


#define TOKENS         \
   X(NoneYet)          \
   X(KeywordType)      \
   X(IntegerType)      \
   X(Identifier)       \
   X(Assignment)       \
   X(IntegerLiteral)   \
   X(FloatLiteral)     \
   X(StringLiteral)    \
   X(BooleanLiteral)   \
   X(Comment)          \
   X(MoreThan)         \
   X(LessThan)         \
   X(MoreOrEqual)      \
   X(LessOrEqual)      \
   X(Equal)            \
   X(Inequal)          \
   X(Negation)         \
   X(PlusOperator)     \
   X(MinusOperator)    \
   X(MultOperator)     \
   X(If)               \
   X(Else)             \
   X(ParenOpen)        \
   X(ParenClose)       \
   X(BracketOpen)      \
   X(BracketClose)     \
   X(Comma)            \
   X(AndOperator)      \
   X(ModOperator)      \
   X(EndOfTokens)      \
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

#define LEX_RESULT_CODE  \
   X(AllGood)            \
   X(DifferentExpected)  \
   X(Missing)            \
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
   size_t start_line;   // 1 indexed
   size_t start_column; // 1 indexed
};

bool
is_alpha(char character)
{
   bool is_uppercase_ascii = character >= 65 && character <= 90;
   bool is_lowercase_ascii = character >= 97 && character <= 122;

   if(is_uppercase_ascii || is_lowercase_ascii)
   {
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

   size_t current_file_line_1indexed;
   size_t current_file_column_1indexed;
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

   lexer->current_file_line_1indexed = 1;
   lexer->current_file_column_1indexed = 1;

   return lexer;
}

/* does not deallocate data under received pointer to characters */
void
lexer_free(struct Lexer* self)
{
   vector_free(self->tokens);
   free(self);
}

void
lexer_save_token(struct Lexer* self, struct Token token)
{
   size_t token_start_line   = self->current_file_line_1indexed;
   size_t token_start_column = self->current_file_column_1indexed - token.len;

   ASSERT(token_start_column >= 1);
   ASSERT(token_start_line   >= 1);

   token.start_line   = token_start_line;
   token.start_column = token_start_column;

   vector_append(self->tokens, &token);
}

void
lexer_advance(struct Lexer* self)
{
   self->current_file_column_1indexed++;
   if(self->current_character == '\n')
   {
      self->current_file_line_1indexed++;
      self->current_file_column_1indexed = 1;
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

const char *const keywords[] =
{
   "int",
   "float",
   "string",
   "bool",
   "if",
   "else",
   "print",
   "while",
   "return"
};

size_t keywords_length = sizeof(keywords) / sizeof(keywords[0]);

const char *const bools[] =
{
   "true",
   "false",
};
size_t bools_length= sizeof(bools) / sizeof(bools[0]);

void
lexer_identifier_or_keyword(struct Lexer* self)
{
   char*  start_ptr               = self->characters + self->current_index;
   size_t lexeme_length           = 1; // single char initially
   struct Token established_token;
   lexer_advance(self);

   while(!self->end_of_characters && ('_' == self->current_character || is_alpha(self->current_character) || is_numeric(self->current_character)))
   {
      ++lexeme_length; 
      lexer_advance(self);
   }

   for(size_t i = 0; i<keywords_length; i++)
   {
      const char* current_keyword = keywords[i];
      size_t keyword_length       = strlen(current_keyword);

      if(keyword_length != lexeme_length) { continue; }

      if(0 == strncmp(current_keyword, start_ptr, lexeme_length))
      {
         established_token.value_ptr = start_ptr;
         established_token.len       = lexeme_length;
         established_token.kind      = KeywordType;

         if(lexeme_length == 3 && 0 == strncmp("int", start_ptr, 3))
         {
            established_token.kind = IntegerType;
         }

         lexer_save_token(self, established_token);         
         return;
      }
   }

   for(size_t i = 0; i<bools_length; i++)
   {
      const char* current_bool_keyword = bools[i];
      size_t bool_length       = strlen(current_bool_keyword);

      if(bool_length != lexeme_length) { continue; }

      if(0 == strncmp(current_bool_keyword, start_ptr, lexeme_length))
      {
         established_token.value_ptr = start_ptr;
         established_token.len       = lexeme_length;
         established_token.kind      = BooleanLiteral;

         lexer_save_token(self, established_token);         
         return;
      }
   }

   established_token.value_ptr = start_ptr;
   established_token.len       = lexeme_length;
   established_token.kind      = Identifier;

   lexer_save_token(self, established_token);
}

struct LexResult
lexer_number(struct Lexer* self)
{
   char*  start_ptr            = self->characters + self->current_index;
   size_t lexeme_length        = 1; // single char initially

   // integer literal
   lexer_advance(self);
   while(!self->end_of_characters && is_numeric(self->current_character))
   {
      ++lexeme_length; 
      lexer_advance(self);
   }

   struct Token identified_token =
   {
     .value_ptr = start_ptr,
     .len       = lexeme_length,
     .kind      = IntegerLiteral,
   };

   // float literal expansion 
   if(self->current_character == '.')
   {
      lexer_advance(self);
      lexeme_length += 1;

      if(!is_numeric(self->current_character))
      {      
         static char error_message[256];
         snprintf
         (
            error_message, sizeof(error_message),
            "unexpected character '%c' on line %zu, column %zu following float dot '.', expected digits to follow",
            self->current_character, self->current_file_line_1indexed, self->current_file_column_1indexed
         );

         return
         (struct LexResult)
         {
            .code = DifferentExpected,
            .error_message = error_message
         };
      }

      while(!self->end_of_characters && is_numeric(self->current_character))
      {
         ++lexeme_length; 
         lexer_advance(self);         
      }

      identified_token.kind      = FloatLiteral;
      identified_token.len       = lexeme_length;
      identified_token.value_ptr = start_ptr;
      
   }

   lexer_save_token(self, identified_token);

   return
   (struct LexResult)
   {
      .code = AllGood,
      .error_message = NULL
   };
}

struct LexResult
lexer_comment(struct Lexer* self)
{
   char* start_ptr = self->characters + self->current_index;
   size_t lexeme_length = 1; // single char initially
   struct Token established_token;

   lexer_advance(self);

   if(self->current_character != '/')
   {     
      static char error_message[256];
      snprintf
      (
         error_message, sizeof(error_message),
         "unexpected character '%c' on line %zu, column %zu following comment start '/'. expected another '/'",
         self->current_character, self->current_file_line_1indexed, self->current_file_column_1indexed
      );

      return
      (struct LexResult)
      {
         .code = DifferentExpected,
         .error_message = error_message
      };
   }

   while(!self->end_of_characters && self->current_character != '\n')
   {
      ++lexeme_length;
      lexer_advance(self);
   }

   established_token.value_ptr = start_ptr;
   established_token.len       = lexeme_length;
   established_token.kind      = Comment;

   lexer_save_token(self, established_token);

   return
   (struct LexResult)
   {
      .code          = AllGood,
      .error_message = NULL,
   };
}

struct LexResult
lexer_string(struct Lexer* self)
{
   char* start_ptr = self->characters + self->current_index;
   size_t lexeme_length = 1; // single char initially
   struct Token established_token;

   lexer_advance(self);

   while(!self->end_of_characters && self->current_character != '"')
   {
      ++lexeme_length;
      lexer_advance(self);
   }

   if(self->current_character != '"')
   {
      static char error_message[256];
      snprintf
      (
         error_message, sizeof(error_message),
         "missing string ending for string starting at line %zu, column %zu",
         self->current_file_line_1indexed, self->current_file_column_1indexed
      );
      return
      (struct LexResult)
      {
         .code          = Missing,
         .error_message = error_message,
      };
      
   }
   // NOTE: include enclosing quote into lexeme, kinda messy control flow but to be refactored later
   lexer_advance(self); 
   ++lexeme_length; 

   established_token.value_ptr = start_ptr;
   established_token.len       = lexeme_length;
   established_token.kind      = StringLiteral;

   lexer_save_token(self, established_token);

   return
   (struct LexResult)
   {
      .code          = AllGood,
      .error_message = NULL,
   };
}


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
   char magic_compiler_blocker = '\\';
   
   while(!(self->end_of_characters))
   {
      if(self->current_character == magic_compiler_blocker)
      {
         break;
      }

      if(self->current_character == ' '
         ||
         self->current_character == '\n'
         ||
         self->current_character == '\t')
      {
         lexer_advance(self);
         continue;
      }

      if(self->current_character == ';')
      {
         struct Token identifier_token =
         {
           .value_ptr = self->characters + self->current_index,
           .len       = 1,
           .kind      = StatementEnd,           
         };
         lexer_save_token(self, identifier_token);
         lexer_advance(self);
         continue;
      }

      if(self->current_character == '/')
      {
         struct LexResult result = lexer_comment(self);
         if(result.code != AllGood) { return result; }
         continue;
      }

      if(self->current_character == '"')
      {
         struct LexResult result = lexer_string(self);
         if(result.code != AllGood) { return result; }
         continue;
      }      

      if(is_alpha(self->current_character))
      {
         lexer_identifier_or_keyword(self);
         continue;
      }

      if(is_numeric(self->current_character))
      {
         struct LexResult result = lexer_number(self);
         if(result.code != AllGood) { return result; }
         continue;
      }

      if(self->current_character == '>')
      {
         char* lexeme_ptr = self->characters + self->current_index;
         struct Token established_token;

         lexer_advance(self);

         if(self->current_character == '=')
         {
            established_token.len        = 2;
            established_token.kind       = MoreOrEqual;
            established_token.value_ptr  = lexeme_ptr;
            // prepare the next one
            lexer_advance(self);
         }
         else
         {
            established_token.len        = 1;
            established_token.kind       = MoreThan;
            established_token.value_ptr  = lexeme_ptr;                    
         }
         lexer_save_token(self, established_token);
         continue;
      }

      if(self->current_character == '=')
      {
         char* lexeme_ptr = self->characters + self->current_index;
         struct Token established_token;

         lexer_advance(self);

         if(self->current_character == '=')
         {
            established_token.len        = 2;
            established_token.kind       = Equal;
            established_token.value_ptr  = lexeme_ptr;
            // prepare the next one
            lexer_advance(self);
         }
         else
         {
            established_token.len        = 1;
            established_token.kind       = Assignment;
            established_token.value_ptr  = lexeme_ptr;            
         }
         lexer_save_token(self, established_token);
         continue;
      }      

      if(self->current_character == '<')
      {
         char* lexeme_ptr = self->characters + self->current_index;
         struct Token established_token;

         lexer_advance(self);

         if(self->current_character == '=')
         {
            established_token.len        = 2;
            established_token.kind       = LessOrEqual;
            established_token.value_ptr  = lexeme_ptr;
            // prepare the next one
            lexer_advance(self);
         }
         else
         {
            established_token.len        = 1;
            established_token.kind       = LessThan;
            established_token.value_ptr  = lexeme_ptr;                    
         }
         lexer_save_token(self, established_token);
         continue;
      }

      if(self->current_character == '+')
      {
         struct Token established_token =
         {
            .kind      = PlusOperator,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }

      if(self->current_character == '-')
      {
         struct Token established_token =
         {
            .kind      = MinusOperator,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }

      if(self->current_character == '*')
      {
         struct Token established_token =
         {
            .kind      = MultOperator,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }

      if(self->current_character == '%')
      {
         struct Token established_token =
         {
            .kind      = ModOperator,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }      
      
      if(self->current_character == '(')
      {
         struct Token established_token =
         {
            .kind      = ParenOpen,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }

      if(self->current_character == ')')
      {
         struct Token established_token =
         {
            .kind      = ParenClose,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }      

      if(self->current_character == '{')
      {
         struct Token established_token =
         {
            .kind      = BracketOpen,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }

      if(self->current_character == '}')
      {
         struct Token established_token =
         {
            .kind      = BracketClose,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }

      if(self->current_character == ',')
      {
         struct Token established_token =
         {
            .kind      = Comma,
            .len       = 1,
            .value_ptr = (char*)self->characters + self->current_index
         };

         lexer_save_token(self, established_token);
         lexer_advance(self); // prepare next
         continue;
      }      

      if(self->current_character == '!')
      {
         struct Token established_token;
         char* value_ptr = self->characters + self->current_index;

         lexer_advance(self);
         if('=' == self->current_character)
         {
            established_token.kind      = Inequal;
            established_token.len       = 2;
            established_token.value_ptr = value_ptr;
            lexer_advance(self); // prepare next
         }
         else
         {
            established_token.kind      = Negation;
            established_token.len       = 1;
            established_token.value_ptr = value_ptr;
         }
            
         lexer_save_token(self, established_token);
         continue;
      }      

      if('&' == self->current_character)
      {
         char* value_ptr = (char*)self->characters + self->current_index;
         lexer_advance(self);

         if('&' == self->current_character)
         {
            struct Token established_token =
            {
               .kind      = AndOperator,
               .len       = 2,
               .value_ptr = value_ptr
            };            
            lexer_save_token(self, established_token);
            lexer_advance(self); // prepare next
            continue;
         }

         static char error_message[256];
         snprintf
         (
            error_message, sizeof(error_message),
            "unexpected character '%c' encountered on line %zu, column %zu. '&' should be followed by another '&' to create an AND statement",
            self->current_character, self->current_file_line_1indexed, self->current_file_column_1indexed
         );

         return
         (struct LexResult)
         {
         .code          = DifferentExpected,
         .error_message = error_message
         };
      }      
      
      static char error_message[256];
      snprintf
      (
         error_message, sizeof(error_message),
         "invalid character '%c' encountered on line %zu, column %zu",
         self->current_character, self->current_file_line_1indexed, self->current_file_column_1indexed
      );

      return
      (struct LexResult)
      {
      .code          = InvalidCharacter,
      .error_message = error_message
      };
   }

   struct Token end_of_tokens =
   {
      .kind         = EndOfTokens,
      .value_ptr    = NULL,
      .len          = 0,
      .start_line   = self->current_file_line_1indexed,
      .start_column = self->current_file_column_1indexed
   };
   lexer_save_token(self, end_of_tokens);

   return
   (struct LexResult)
   {
   .code          = AllGood,
   .error_message = NULL
   };
}

/* part parser */
#define NODE_TYPES           \
        X(NodeScope)         \
        X(NodeVariableBind)  \
        X(NodeInteger)       \
        X(NodeBinary)

enum NodeKind
{
   #define X(name) name,
   NODE_TYPES
   #undef X
};

char*
node_kind_to_str(enum NodeKind kind)
{
   #define X(name) if(kind == name) { static char* variable = #name; return variable; }
   NODE_TYPES
   #undef X

   ASSERT("no execution path leads here" && false);
}

struct Node
{
   enum NodeKind kind;
   struct Token* token_ptr;

   // NodeScope
   struct vector* nodes;
   // NodeScope

   // NodeVariableBind
   struct Node* value_expr;
      // should have token_ptr;
   enum TokenKind type;
   // NodeVariableBind

   // NodeBinary
   enum TokenKind operand;
   struct Node* left;
   struct Node* right;
   // NodeBinary

   // NodeInteger
      // should have token_ptr;
   // NodeInteger
};

struct Node*
node_create()
{
   // NOTE/TODO: suboptimal, memory arena maybe?
   struct Node* node = malloc(sizeof(struct Node));
   ASSERT(node != NULL);

   return node;
}

void
node_tree_debug(struct Node* node, size_t current_tree_level)
{
   printf("\n");
   for(size_t i = 0; i < current_tree_level; i++)
   {
      printf("\t");
   }

   printf("NodeKind: %s | level %zu", node_kind_to_str(node->kind), current_tree_level);
   switch(node->kind)
   {
     case NodeScope:
        {
            for(size_t i = 0; i < node->nodes->element_count; i++)
            {
                struct Node* sub_node = vector_get(node->nodes, i);
                node_tree_debug(sub_node, current_tree_level + 1);
            }
            break;
        }
      case NodeVariableBind:
         {
            printf(" | type: ", token_kind_to_str(node->type));
            node_tree_debug(node->value_expr, current_tree_level + 1);
            break;
         }

      case NodeBinary:
         {
            printf(" | op: %s", token_kind_to_str(node->operand));
            node_tree_debug(node->left, current_tree_level + 1);
            node_tree_debug(node->right, current_tree_level + 1);
            break;
         }
      case NodeInteger:
         {
            printf(" | value: %.*s", (int)node->token_ptr->len, node->token_ptr->value_ptr);
            break;
         }         

   }
}

struct Parser
{
   struct vector* tokens;
   struct Token*  current_token;
   size_t         token_index;
   struct Node*   root_node; // NodeScope
};


struct Parser*
parser_create(struct vector* tokens)
{
   // parser init
   struct Parser*  parser = malloc(sizeof(struct Parser));
   ASSERT(NULL != parser);

   parser->tokens = tokens;

   return parser;
}

// NOTE/TODO: implement actual freeing
void
parser_free()
{
   return;
}

void
parser_advance(struct Parser* self)
{
   ASSERT(self->token_index + 1 < self->tokens->element_count);

   self->token_index = self->token_index + 1;
   self->current_token = vector_get(self->tokens, self->token_index);
}

void
parser_assert(struct Parser* self, enum TokenKind expected_kind)
{
   if(self->current_token->kind != expected_kind)
   {
      fprintf
      (
        stderr,
        "[PARSE ERROR] expected '%s', received '%s' at line %zu, column %zu\n",
        token_kind_to_str(expected_kind), token_kind_to_str(self->current_token->kind),
        self->current_token->start_line, self->current_token->start_column
      );
      abort();
   }
}

bool
parser_accept(struct Parser* self, enum TokenKind expected_kind)
{
   if(self->current_token->kind == expected_kind)
   {
      return true;
   }

   return false;
}

struct Node*
parser_parse_expression(struct Parser* self)
{
   if(parser_accept(self, IntegerLiteral))
   {
      struct Node* number_node = node_create();
      number_node->kind        = NodeInteger;
      number_node->token_ptr   = self->current_token;

      // NOTE/TODO: probably should drop reliance on
      // token_tpr data and encapsulate this into parser_accept.
      // right now current token data needs to be saved before proceeding
      parser_advance(self);

      if(parser_accept(self, MinusOperator))
      {
         parser_advance(self);
         struct Node* bin_node   = node_create();
         struct Node* expression = parser_parse_expression(self);

         bin_node->kind    = NodeBinary;
         bin_node->left    = number_node;
         bin_node->right   = expression;
         bin_node->operand = MinusOperator;

         return bin_node;
      }
      else if(parser_accept(self, PlusOperator))
      {
         parser_advance(self);
         struct Node* bin_node   = node_create();
         struct Node* expression = parser_parse_expression(self);

         bin_node->kind    = NodeBinary;
         bin_node->left    = number_node;
         bin_node->right   = expression;
         bin_node->operand = PlusOperator;

         return bin_node;
      }

      return number_node;
   }

   return NULL;
}

struct Node*
parser_parse_variable_bind(struct Parser* self)
{
      struct Node* node = node_create();
      node->kind = NodeVariableBind;

      parser_assert(self, IntegerType);
      node->type = IntegerType;
      parser_advance(self);

      parser_assert(self, Identifier);
      node->token_ptr = self->current_token;
      parser_advance(self);

      parser_assert(self, Assignment);
      parser_advance(self);

      struct Node* expression = parser_parse_expression(self);
      node->value_expr = expression;

      parser_assert(self, StatementEnd);
      parser_advance(self);

      return node;
}

void
parser_parse(struct Parser* self)
{
   // root_node init
   struct Node*    root_node = node_create();
   struct vector*  nodes     = vector_create(sizeof(struct Node));
   root_node->kind   = NodeScope;
   root_node->nodes  = nodes;
   self->root_node = root_node;

   // initialize first token
   ASSERT(self->tokens->element_count > 0);
   self->token_index = 0;
   self->current_token = vector_get(self->tokens, self->token_index);

   while(self->current_token->kind != EndOfTokens)
   {
      struct Node* node = parser_parse_variable_bind(self);
      if(node != NULL)
      {
         vector_append(self->root_node->nodes, node);
      }
   }
}

/* part main */
int
main(void)
{
   // read the file
   FILE* compiled_file = fopen("./delight-editor/start_parser", "r");

   if(NULL == compiled_file)
   {
      printf("file opening has failed");
      return -1;
   }

   char file_content_buffer[4096];

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
       "kind '%s' | value: '%.*s' | length: %zu\n",
       token_kind_to_str(token->kind),
       (int)token->len, (char*)token->value_ptr, token->len
      );
   }

   // (p)arse
   struct Parser* parser = parser_create(lexer->tokens);
   parser_parse(parser);

   size_t current_tree_level = 0;
   node_tree_debug(parser->root_node, current_tree_level);

   // (c)lean(up)
   parser_free(); // NOTE/TODO: implement actual free

   // NOTE/TODO: parser relies on lexers tokens for certain nodes
   lexer_free(lexer);

   return 0;
}
