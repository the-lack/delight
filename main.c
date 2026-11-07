#include <stdio.h>

int
main(void)
{
   FILE* compiled_file = fopen("./delight-editor/start", "r");

   if(NULL == compiled_file)
   {
      printf("opening the compiled file has failed");
      return -1;
   }

   char file_content_buffer[1024];

   size_t fread_ret = fread(file_content_buffer, sizeof(char), sizeof(file_content_buffer), compiled_file);
   if(0 != ferror(compiled_file))
   {
      printf("loading the compiled file into memory has failed");
      return -1;
   }

   fwrite(file_content_buffer, sizeof(char), fread_ret, stdout);
}
