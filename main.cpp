#include <algorithm>
#include <cstdio>
#include <ios>
#include <print>
#include <fstream>
#include <cassert>
#include <vector>

using std::string;

// what tokens are there?
// type annotation = int | string
// identifier      = dynamic "a" | "b" | "c" | "d"
// string
//
// we could go over char buffer[] and group together related tokens
// we should go as far as possible meaning that we loop
//
// found_tokens = []
// delimiters = EOF, \n, \r\n ;, space
// 
// current_token = ""
// for char in buffer {
//            if current_token IN set of delimiters { // we cannot add delimiter as valid grammar but this also tells us about token end
//                 if(current token IN set of valid tokens) found_tokens.push currenttoken
//                 else(current token IN set of valid tokens) 
//            }
//            current_token += char
//
//            if current_token IN set of valid tokens {
//                  // we could pick up current token as valid already but we need to make sure the next thing is a space
//                  // otherwise me might try <int> type annotaiton and <integers_are_my_fav> as the same thing (both start as <int>)
//
//                  therefore expansion of the token is required
//                  problem with expansion is the fact that the next token will or won't be a valid/good token
//
//                  e.g. str & string are two valid variations but
//                       str & stribidisigma are not. we need to always be able to expand to as much as possible
//                       actually this is not that big of a problem given the fact that <space>whaterver<space> is the thing. i guess.
// 
//                  the condition actually could be in the loop -> while char not in delimiters or something like that.
//                  if its a delimiter we need to take previous token and name/label it
//
//                  i guess initial token types will be <SPACE> (needed for separation). actually no, we do not need it tbh as a token.
//                  i guess ; would be nice but not really tbh.
//
//                  i guess: (+, -, *, /) math operators. (maybe reserved sum, subtract, times, divide)
//                  = which is an assignment operator
//                  type declaration, only int exists
//                  value declaration, only numbers exist, they have to have int format without commabs just [0-9]*n
//                  variable declaration, simple variable naming format which is a-z, that's it.
//
//                  certain grammar structure: <type dec> <variable name> <assignment> <value>.
//                  actually <value> needs to be an expression.
//                  does it?
//
//                  do I want my language to have int abc = 1 + 1;?
//
//                  yeah i guess this makes sense. generally this also forces to think about associativity rules.
//                  mathematical associativity rules are quite complex actually.
//                  i could actually implement zero associativity rules and force use to implement them by using brackets ((1 * 1) + 1)
//                  this implies tho that bracket expression have the priority. hmmm. that might make sense. do i like bracket syntax?
//                  bracket syntax is very mid imho. i could make everything left-associative. everything. breaking math rules but introducing
//                  permanent single rule over everything.
//
//              
//
//                  i guess expression is vritually whatever combined with other whaterver.
//                  e.g.: <value> <operator> <another value>
//                  how many values can there be?
//
// 
// 
//            }
// 
// }
// 
// there needs to be grouping rules.


enum class TokenKind {
  None = 0,
  Operator
};

struct Token {
  TokenKind kind = TokenKind::None;
  // TODO: use string view or sth else to avoid heap allocations/reallocations - all token values are already in the file buffer
  string value = {}; 
};

const string entry_point_path = "./delight-editor/start";

void TODO(string description, bool condition_is_ok) {
  if(condition_is_ok) return;

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

  // tokenize
  std::vector<string> found_tokens= {  };

  std::vector<char> character_allow_list = {
    // ascii characters
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'r', 's', 't', 'u', 'w', 'x', 'y', 'z',

    // numbers
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

    // delimiters
    ' ', '\n', '\r',

    // operators
    '='
    
  };

  string current_token = {};
  for(char character : file_vector) {

    if(!std::ranges::contains(character_allow_list, character)) {
        std::println(stderr, "ERROR: invalid character found during compilation: {}", character);
        std::exit(EXIT_FAILURE);     
    }
    
    if(character == ' ' || character == '\r' || character == '\n') {
      found_tokens.push_back(current_token);
      current_token = {};
      continue;
    }

    current_token += character;
  }

  std::println("tokenization went right, here's your list of tokens dear sir {}", found_tokens);
  entrypoint_file_stream.close();

  // parse me daddy
  
  return 0;
}
  
