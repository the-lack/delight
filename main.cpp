#include <ios>
#include <print>
#include <fstream>
#include <cassert>
#include <vector>

using std::string;

const string entry_point_path = "./delight-editor/start";

void TODO(string description, bool when_false_causes_exit) {
  if(when_false_causes_exit == true) return;

  std::println(stderr, "EXIT_FAILURE [TODO]: {}", description);
  std::exit(EXIT_FAILURE);     
}

void ASSERT_THAT(string description, bool when_false_causes_exit) {
  if(when_false_causes_exit == true) return;

  std::println(stderr, "EXIT_FAILURE [ASSERT]: {}", description);
  std::exit(EXIT_FAILURE);     
}

int main() {
  std::println("start");

  std::fstream entrypoint_file_stream;
  entrypoint_file_stream.open(entry_point_path, std::ios::in);

  {
  bool file_stream_opening_did_not_fail = !entrypoint_file_stream.fail();
  TODO("graceful handling of (lack of permissions) | (non-existent file) | (non-existent path) | (other stuff)", file_stream_opening_did_not_fail);
  }

  // calculate file size in bytes
  entrypoint_file_stream.seekg(0, std::ios::end);
  std::streampos file_size = entrypoint_file_stream.tellg();
  entrypoint_file_stream.seekg(0, std::ios::beg);

  
  ASSERT_THAT("file size is not negative", file_size >= 0);
  auto file_size_in_bytes = static_cast<std::size_t>(file_size);

  // pull data into vector
  std::vector<char> file_vector(file_size_in_bytes);
  char* const vector_pointer = file_vector.data();
  entrypoint_file_stream.read(vector_pointer, file_size);

  entrypoint_file_stream.close();
  
  return 0;
}

