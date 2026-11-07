#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

   fwrite(file_content_buffer, sizeof(char), fread_file_byte_count, stdout);

   // (f)lex
   struct vector* current_lexeme = vector_create(sizeof(char));

   for(size_t i = 0; i < fread_file_byte_count; i++) {
      char character = file_content_buffer[i];

      vector_append(current_lexeme, &character);      
   }   

   for(size_t i = 0; i < current_lexeme->element_count; i++){
      printf("%c", *((char*)vector_get(current_lexeme, i)));
   }

   vector_free(current_lexeme);

   
   return 0;
}
