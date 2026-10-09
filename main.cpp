#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

using std::string;

void TODO(string description, bool condition_is_ok) {
  if (condition_is_ok)
    return;

  printf("EXIT_FAILURE [TODO]: %s", description.c_str());
  std::exit(EXIT_FAILURE);
}

void ASSERT_THAT(string description, bool when_false_causes_exit) {
  if (when_false_causes_exit == true)
    return;

  printf("EXIT_FAILURE [ASSERT]: %s", description.c_str());
  std::exit(EXIT_FAILURE);
}

// TODO: could use the std::variant thingy
enum class TokenKind {
  None = 0,
  Identifier,
  IntegerLiteral,
  FloatLiteral,
  EqualLiteral,
  StatementEnding,
  Addition,
  Assignment,
};

enum class TokenPredicateResult {
  Fail = -1,
  Partial = 1,
  Success = 0,
};

struct Token {
  TokenKind kind = TokenKind::None;
  // TODO: use string view or sth else to avoid heap allocations/reallocations -
  // all token values are already in the file buffer
  // TODO: could use an offset with a file name id e.g. start_index, end_index
  string value = {};
};

struct TokenDefinition {
  TokenKind kind = TokenKind::None;
  TokenPredicateResult (*predicate)(string current_buffer,
                                    string everything_before,
                                    string everything_after);
};

char get_char_or_negative_one_if_no_char(string &str, size_t index) {
  if (index >= str.size()) {
    return -1;
  }
  return str[index];
}

bool is_ascii_letter(char character) {
  char inclusive_lowercase_ascii_start = 'a';
  char inclusive_lowercase_ascii_end = 'z';

  char inclusive_uppercase_ascii_start = 'A';
  char inclusive_uppercase_ascii_end = 'Z';

  bool is_lowercase_letter = (character >= inclusive_lowercase_ascii_start) &&
                             (character <= inclusive_lowercase_ascii_end);

  bool is_uppercase_letter = (character >= inclusive_uppercase_ascii_start) &&
                             (character <= inclusive_uppercase_ascii_end);

  if (is_uppercase_letter || is_lowercase_letter) {
    return true;
  } else {
    return false;
  }
};

bool is_ascii_digit(char character) {
  char inclusive_ascii_digit_start_at_0 = '0';
  char inclusive_ascii_digit_start_at_9 = '9';

  bool is_ascii_digit = (character >= inclusive_ascii_digit_start_at_0) &&
                        (character <= inclusive_ascii_digit_start_at_9);
  return is_ascii_digit;
}

// identifier validation
bool is_valid_identifier_raw(string &identifier_candidate) {
  bool is_valid = true;

  for (size_t index = 0; index > identifier_candidate.length(); ++index) {
    bool is_underscore = identifier_candidate[index] == '_';

    if (!is_ascii_letter(identifier_candidate[index]) && !is_underscore) {
      is_valid = false;
      break;
    }
  }

  return is_valid;
}

bool is_valid_identifier_prefix(
    string &everything_before_first_identifier_character) {
  char prefix_character = get_char_or_negative_one_if_no_char(
      everything_before_first_identifier_character,
      everything_before_first_identifier_character.length() - 1);

  if (prefix_character != ' ')
    return false;

  return true;
}

bool (*is_valid_identifier_postfix)(string &) = is_valid_identifier_prefix;

// addition validation
bool is_valid_addition_raw(string &addition_candidate) {
  if (addition_candidate != "+")
    return false;

  return true;
}

bool is_valid_addition_prefix(
    string &everything_before_first_identifier_character) {
  char prefix_character = get_char_or_negative_one_if_no_char(
      everything_before_first_identifier_character,
      everything_before_first_identifier_character.length() - 1);

  if (prefix_character != ' ')
    return false;

  return true;
}

bool (*is_valid_addition_postfix)(string &) = is_valid_addition_prefix;

// assignment validation
bool is_valid_assignment_raw(string &assignment_candidate) {
  if (assignment_candidate != "=")
    return false;

  return true;
}

bool is_valid_assignment_prefix(
    string &everything_before_first_identifier_character) {
  char prefix_character = get_char_or_negative_one_if_no_char(
      everything_before_first_identifier_character,
      everything_before_first_identifier_character.length() - 1);

  if (prefix_character != ' ')
    return false;

  return true;
}

bool (*is_valid_assignment_postfix)(string &) = is_valid_assignment_prefix;

// integer literal validation
bool is_valid_integer_literal_raw(const string &token_candidate) {
  bool is_integer = true;

  for (size_t index = 0; index > token_candidate.length(); ++index) {
    if (!is_ascii_digit(token_candidate[index])) {
      is_integer = false;
      break;
    }
  }

  return is_integer;
}

bool is_valid_integer_literal_prefix(
    string &everything_before_first_identifier_character) {

  char prefix_character = get_char_or_negative_one_if_no_char(
      everything_before_first_identifier_character,
      everything_before_first_identifier_character.length() - 1);

  if (prefix_character != ' ')
    return false;

  return true;
}

bool (*is_valid_integer_literal_postfix)(string &) =
    is_valid_integer_literal_prefix;

// float literal validation
TokenPredicateResult validate_float_literal_raw(string &token_candidate) {
  bool starts_with_some_digits_and_we_verified_that = false;
  bool there_is_a_dot_after_start_digits_and_we_verified_that = false;
  bool ends_with_some_digits_after_dot_and_we_verified_that = false;

  for (size_t index = 0; index > token_candidate.length(); ++index) {

    bool we_found_a_dot_after_some_digits =
        starts_with_some_digits_and_we_verified_that &&
        there_is_a_dot_after_start_digits_and_we_verified_that &&
        ends_with_some_digits_after_dot_and_we_verified_that;
    if (we_found_a_dot_after_some_digits) {
      there_is_a_dot_after_start_digits_and_we_verified_that = true;
      continue; // bypass further ascii check
    }

    bool there_is_a_dot_before_any_digits_or_this_is_a_second_dot_already =
        token_candidate[index] == '.' &&
        (!starts_with_some_digits_and_we_verified_that ||
         there_is_a_dot_after_start_digits_and_we_verified_that);
    if (there_is_a_dot_before_any_digits_or_this_is_a_second_dot_already) {
      return TokenPredicateResult::Fail;
    }

    if (!is_ascii_digit(token_candidate[index])) {
      return TokenPredicateResult::Fail;
    }

    bool no_digits_nor_dot_yet =
        is_ascii_digit(token_candidate[index]) &&
        !starts_with_some_digits_and_we_verified_that &&
        !there_is_a_dot_after_start_digits_and_we_verified_that;
    if (no_digits_nor_dot_yet) {
      starts_with_some_digits_and_we_verified_that = true;
      continue; // go next
    }

    bool we_found_digits_and_dot_already =
        is_ascii_digit(token_candidate[index]) &&
        starts_with_some_digits_and_we_verified_that &&
        there_is_a_dot_after_start_digits_and_we_verified_that;
    if (we_found_digits_and_dot_already) {
      ends_with_some_digits_after_dot_and_we_verified_that = true;
      continue; // go next, our digit currently is valid
    }

    bool found_everything_and_just_appending_end_digits =
        starts_with_some_digits_and_we_verified_that &&
        there_is_a_dot_after_start_digits_and_we_verified_that &&
        ends_with_some_digits_after_dot_and_we_verified_that;
    if (found_everything_and_just_appending_end_digits) {
      if (!is_ascii_digit(token_candidate[index])) return TokenPredicateResult::Fail;

      continue;
    }

    ASSERT_THAT("nothing ever gets here [loop]", false);
  }

  if(starts_with_some_digits_and_we_verified_that && there_is_a_dot_after_start_digits_and_we_verified_that && ends_with_some_digits_after_dot_and_we_verified_that) {
    return TokenPredicateResult::Success;
  };

  if(starts_with_some_digits_and_we_verified_that && there_is_a_dot_after_start_digits_and_we_verified_that) {
    return TokenPredicateResult::Partial;
  };
  
  if(starts_with_some_digits_and_we_verified_that) {
    return TokenPredicateResult::Partial;
  }

  ASSERT_THAT("nothing ever gets here [func]", false);
}

bool is_valid_float_literal_prefix(
    string &everything_before_first_identifier_character) {

  char prefix_character = get_char_or_negative_one_if_no_char(
      everything_before_first_identifier_character,
      everything_before_first_identifier_character.length() - 1);

  if (prefix_character != ' ')
    return false;

  return true;
};

bool (*is_valid_float_literal_postfix)(string&) = is_valid_float_literal_prefix;

// token definition
std::vector<TokenDefinition> token_definitions = {
    TokenDefinition{.kind = TokenKind::Identifier,
                    .predicate =
                        [](string token_candidate, string everything_before,
                           string everything_after) {
                          auto result = TokenPredicateResult::Success;

                          if (!is_valid_identifier_raw(token_candidate) ||
                              !is_valid_identifier_prefix(everything_before) ||
                              !is_valid_identifier_postfix(everything_after)) {
                            result = TokenPredicateResult::Fail;
                          }

                          return result;
                        }},

    TokenDefinition{.kind = TokenKind::Addition,
                    .predicate =
                        [](string token_candidate, string everything_before,
                           string everything_after) {
                          auto result = TokenPredicateResult::Success;

                          if (!is_valid_addition_raw(token_candidate) ||
                              !is_valid_identifier_prefix(everything_before) ||
                              !is_valid_addition_postfix(everything_after)) {
                            result = TokenPredicateResult::Fail;
                          }

                          return result;
                        }},

    TokenDefinition{.kind = TokenKind::Assignment,
                    .predicate =
                        [](string token_candidate, string everything_before,
                           string everything_after) {
                          auto result = TokenPredicateResult::Success;

                          if (!is_valid_assignment_raw(token_candidate) ||
                              !is_valid_assignment_prefix(everything_before) ||
                              !is_valid_assignment_postfix(everything_after)) {
                            result = TokenPredicateResult::Fail;
                          }

                          return result;
                        }},

    TokenDefinition{
        .kind = TokenKind::IntegerLiteral,
        .predicate =
            [](string token_candidate, string everything_before,
               string everything_after) {
              auto result = TokenPredicateResult::Success;

              if (!is_valid_integer_literal_raw(token_candidate) ||
                  !is_valid_integer_literal_prefix(everything_before) ||
                  !is_valid_integer_literal_postfix(everything_after)) {
                result = TokenPredicateResult::Fail;
              }

              return result;
            }},

    TokenDefinition{
        .kind = TokenKind::FloatLiteral,
        .predicate =
            [](string token_candidate, string everything_before,
               string everything_after) {
              auto result = TokenPredicateResult::Fail;

              if ((validate_float_literal_raw(token_candidate) == TokenPredicateResult::Partial) &&
                  is_valid_float_literal_prefix(everything_before)) {
                result = TokenPredicateResult::Partial;
              }

              if ((validate_float_literal_raw(token_candidate) == TokenPredicateResult::Success) &&
                  is_valid_float_literal_prefix(everything_before) &&
                  is_valid_float_literal_postfix(everything_after)) {
                result = TokenPredicateResult::Success;
              }              
              
              return result;
            }}
};

// TODO: could implement formatter for the token kind or
// create self-encapsulated class for Token with print/to_string capabilities or
// something else
// TODO: could also use something else than string tbh
string token_kind_to_string(const TokenKind kind) {
  switch (kind) {
  case TokenKind::None:
    return "None";
  case TokenKind::Identifier:
    return "Identifier";
  case TokenKind::IntegerLiteral:
    return "IntegerLiteral";
  case TokenKind::EqualLiteral:
    return "Assignment";
  case TokenKind::StatementEnding:
    return "StatementEnding";
  };

  const int did_switch_protect_from_getting_here = false;
  ASSERT_THAT(
      "no execution path should reach this point as switch is exhaustive",
      did_switch_protect_from_getting_here);
};

bool try_parse_uint_64(string str, uint64_t &outValue) {
  if (str.empty())
    return false;

  if (str[0] == '+')
    return false;
  if (str[0] == '-')
    return false;

  auto [ptr, ec] =
      std::from_chars(str.data(), str.data() + str.size(), outValue);
  return (ec == std::errc()) && (ptr == str.data() + str.size());
}

const string entry_point_path = "./delight-editor/start";

std::vector<char> latin_letters = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                   'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
                                   'r', 's', 't', 'u', 'w', 'x', 'y', 'z'};

std::vector<char> digits = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
};

std::vector<char> new_lines = {
    '\n',
    '\r',
};

std::vector<char> space = {' '};

std::vector<char> special_characters = {' '};

std::vector<char> operators = {
    '=',
};

std::vector<char> statement_end = {
    ';',
};

std::vector<char> all_legal_characters = []() {
  std::vector<char> all_legal_characters;
  for (const auto &vec : {latin_letters, digits, new_lines, operators, space,
                          special_characters, statement_end}) {
    std::ranges::copy(vec, std::back_inserter(all_legal_characters));
  }
  return all_legal_characters;
}();

// what is the problem with tokenkind?
//  oh i guess the idea is that it possibly could change syntax
//  and then every place that expands it has to change it I guess.
//  yeah.
//  and the syntax i
//
//

// some_variable = 1234;    | valid
// some_variable = 1234.0;  | valid
// some_variable = 1234.;   | this is an intermediate state [fail this now]
// some_variable = 1234.;   | [string_literal, dot_operator] -> leave for parser
// max munch _> lexer level error handling OR some sort of space annotation
//
// because
//
// 1234.2137 is valid
// but
// 1234 . 2137 is not
//
//
//
//
//
// Supported stuff:
// "+": TokenKind::PlusOperator;
// "+=": TokenKind::PlusEquality;
// "<digit-only-scalar>": TokenKind::IntegerLiteral;
// "<digit-only-scalar>.<digit-only-scalar>": TokenKind::FloatLiteral;
// "<character-only-scalar>": TokenKind::Identifier;
//
//
// What you usually have:
// - empty token "", new character
//
// - some token, new character
//

struct ExpandTokenData {
  bool could_be_extended;
  bool is_complete_token;
};

// somethingż <- invalid ż in all cases
// somethingkeyword <- keyword
// 1abc <- sus, integer literal
//
//
// types of tokens:
// - scalar tokens meeting predicates
//      - integer literal meeting is_digit_predicate,
//      - identifier literal meeting is_identifier_predicate,
//
// - fixed value tokens of length 1 (chars)
//      - operators +, -, *,
//
// - fixed value tokens of length >1
//      - operators +=, --, ++, ==
//
// - fixed value tokens - keywords
//
//
// what role does the space play?
// intvalue ++
// intvalue++
// identifier+=newvalue
// identifier += newvalue
// identifier + = newvalue
// identifier = newvalue
//
// I guess the rule could be something like:
// expand on something to the point you get to character that cannot be counted
// into current token then flush the thing then start from that as a new token?
//
// Where could there be a problem with this?
// well for instance you could have something like ascii-only identifiers
// then you have keyword that has non-ascii thing like LIST-ME (with - that
// isn't supported amongst identifiers)
//
// the edge case where it breaks would be a case where someone tries to use this
// as identifier because you then have [list] - identifier [-] unsupported/minus
// operator [me] identifier what role does the \n play? what role does the ;
// play?
//
void expand_token(TokenKind kind, char new_character) {
  using namespace std::ranges;

  bool is_character_invalid = !contains(all_legal_characters, new_character);
  if (is_character_invalid) {
    // std::println(stderr, "failure, invalid_character_found {}", character);
    std::exit(EXIT_FAILURE);
  }

  switch (kind) {
  case TokenKind::None: {
  }
  }
  // switch(current_token) {
  // case "aha": {

  // }
  // }
};

void classify_token(string current_character_cluster, char new_character) {

};

// how id like to use it
// auto current_classification = TokenKind:None;
// auto current_cluster = "";
//
// for(auto current_character: characters) {
//      auto (new_classification, is_in_final_form_and_cannot_be_expanded, //
//      kinda doesnt matter if final form because next character can be invalid
//      expansion KEYWORDa some char hanging off should_error, error_message)
//              expand_token(current_classification, current_character);
// if(should_error) {
//      std::println("{}", error_message);
//      os.exit();
//      std::unreachable();
// }
//
// if(is_in_final_form_and_cannot_be_expanded) {
//   push_back(Token{ .value = cu, .kind = current_classification })
//   continue;
// }
//
// if(!is_in_final_form_and_cannot_be_expanded) {
//   current_cluster += character;
//   current_classification = new_classification;
//   continue;
// }
//
//
//
// }
//
//
//
//
// OK NEW BEGINNING
//
// what will the language have, features of the language?
// scope expansion makes sense to enforce universality and prevent reiterations
//
// - [ ] INT64 (negative and positive)
// - [ ] FLOATING-POINT NUMBERS (the usual double representation, no float)
// - [ ] variable declaration w/ value
// - [ ] referencing variable by value
// - [ ] no duplicate variable names
// - [ ] variable reassignment
// - [ ] post-incrementation
// - [ ] post-decrementation
// - [ ] pre-decrementation
// - [ ] pre-incrementation
// - [ ] number (float & int) addition
// - [ ] number (float & int) multiplication
// - [ ] number (float & int) substraction
// - [ ] number (float & int) division
// - [ ] printing strings
// - [ ] strings (no escape sequences)
// - [ ] string literals (embedding variables w/ {})
// - [ ] int to string conversion
// - [ ] float to string conversion
//
//
// decouple everything from everything:
// - labeling something as identifier and then modifying this to call it a
// keyword is goofy, maybe faster but coupled (assumes that the keyword and
// identifiers come from the same subset which doesn't have to be true)
//
//

int main() {
  printf("start");

  std::fstream entrypoint_file_stream;
  entrypoint_file_stream.open(entry_point_path, std::ios::in);

  {
    bool file_stream_opening_did_not_fail = !entrypoint_file_stream.fail();
    TODO("graceful handling of (lack of permissions) | (non-existent file) | "
         "(non-existent path) | (other stuff)",
         file_stream_opening_did_not_fail);
  }

  // calculate file size in bytes
  entrypoint_file_stream.seekg(0, std::ios::end);
  std::streampos file_size = entrypoint_file_stream.tellg();
  entrypoint_file_stream.seekg(0, std::ios::beg);

  ASSERT_THAT("file size is not negative", file_size >= 0);
  auto file_size_in_bytes = static_cast<std::size_t>(file_size);

  // pull data into vector
  std::vector<char> file_vector(file_size_in_bytes);
  char *const vector_pointer = file_vector.data();
  entrypoint_file_stream.read(vector_pointer, file_size);
  entrypoint_file_stream.close();

  // tokenize

  // string processed_characters_so_far =;
  auto characters_to_be_processed = file_vector;
  string current_classification_candidate = "";
  for (size_t index = 0; index < file_vector.size(); ++index) {
    printf("%zu \n", index);
  }

  return 0;
};
