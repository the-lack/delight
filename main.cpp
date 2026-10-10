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

enum class TokenDefinitionPredicateResult {
  TokenInvalid = -1,
  TokenIsValidAndShouldBeFlushed = 0,
  TokenIsValidButShouldBeExpanded = 1
};

enum class RawTokenValidationResult {
  TokenIsInvalid = -1,
  TokenIsPartialAndRequiresExpanding = -2,
  TokenIsValidAndCouldBeExpanded = 0,
  TokenIsValidAndCannotBeExpandedFurther = 1,
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
  TokenDefinitionPredicateResult (*predicate)(
      std::vector<char> &current_buffer, std::vector<char> &everything_before,
      std::vector<char> &everything_after);
};

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

// token definition
std::vector<TokenDefinition> token_definitions = {
    TokenDefinition{
        .kind = TokenKind::Identifier,
        .predicate =
            [](std::vector<char> &token_candidate,
               std::vector<char> &everything_before,
               std::vector<char> &everything_after) {
              if (token_candidate.size() == 0) {
                return TokenDefinitionPredicateResult::TokenInvalid;
              }

              // check if valid identifier
              for (size_t index = 0; index < token_candidate.size(); ++index) {
                bool is_underscore = token_candidate[index] == '_';

                if (!is_ascii_letter(token_candidate[index]) &&
                    !is_underscore) {
                  return TokenDefinitionPredicateResult::TokenInvalid;
                }
              }

              // check prefix
              if (everything_before.size() > 0) {
                char prefix_character = everything_before.back();

                if (is_ascii_letter(prefix_character) ||
                    '_' == prefix_character) {
                  return TokenDefinitionPredicateResult::TokenInvalid;
                }
              }

              if (everything_after.size() == 0) {
                return TokenDefinitionPredicateResult::
                    TokenIsValidAndShouldBeFlushed;
              }

              // check if follow up next character w/ lookahead
              if (everything_after.size() > 0) {
                char postfix_character = everything_after[0];

                if (postfix_character == ' ' || postfix_character == '\r' ||
                    postfix_character == '\n') {
                  return TokenDefinitionPredicateResult::
                      TokenIsValidAndShouldBeFlushed;
                }

                if (is_ascii_letter(postfix_character) ||
                    '_' == postfix_character) {
                  return TokenDefinitionPredicateResult::
                      TokenIsValidButShouldBeExpanded;
                }
              }

              return TokenDefinitionPredicateResult::TokenInvalid;
            }},

    TokenDefinition{.kind = TokenKind::Addition,
                    .predicate =
                        [](std::vector<char> &token_candidate,
                           std::vector<char> &everything_before,
                           std::vector<char> &everything_after) {
                          if (token_candidate.size() != 1 ||
                              token_candidate[0] != '+')
                            return TokenDefinitionPredicateResult::TokenInvalid;

                          return TokenDefinitionPredicateResult::
                              TokenIsValidAndShouldBeFlushed;
                        }},

    TokenDefinition{.kind = TokenKind::Assignment,
                    .predicate =
                        [](std::vector<char> &token_candidate,
                           std::vector<char> &everything_before,
                           std::vector<char> &everything_after) {
                          if (token_candidate.size() != 1 ||
                              token_candidate[0] != '=')
                            return TokenDefinitionPredicateResult::TokenInvalid;

                          return TokenDefinitionPredicateResult::
                              TokenIsValidAndShouldBeFlushed;
                        }},

    TokenDefinition{
        .kind = TokenKind::IntegerLiteral,
        .predicate =
            [](std::vector<char> &token_candidate,
               std::vector<char> &everything_before,
               std::vector<char> &everything_after) {
              if (token_candidate.size() == 0) {
                return TokenDefinitionPredicateResult::TokenInvalid;
              }

              for (size_t index = 0; index < token_candidate.size(); ++index) {
                if (!is_ascii_digit(token_candidate[index])) {
                  return TokenDefinitionPredicateResult::TokenInvalid;
                }
              }

              // prefix
              if (everything_before.size() > 0) {
                char prefix_character = everything_before.back();
                if (is_ascii_digit(prefix_character)) {
                  return TokenDefinitionPredicateResult::TokenInvalid;
                }
              }

              // postfix
              if (everything_after.size() > 0) {
                char postfix_character = everything_after[0];
                if (is_ascii_digit(postfix_character)) {
                  return TokenDefinitionPredicateResult::
                      TokenIsValidButShouldBeExpanded;
                }
              }

              return TokenDefinitionPredicateResult::
                  TokenIsValidAndShouldBeFlushed;

              ASSERT_THAT("no one ever gets here integer literal", false);
            }},

    TokenDefinition{
        .kind = TokenKind::FloatLiteral,
        .predicate = [](std::vector<char> &token_candidate,
                        std::vector<char> &everything_before,
                        std::vector<char> &everything_after) {
          if (token_candidate.size() == 0)
            return TokenDefinitionPredicateResult::TokenInvalid;

          bool starts_with_digits = false;
          bool dot_after_start_digits = false;
          bool ends_with_digits_after_dot = false;

          for (size_t index = 0; index < token_candidate.size(); ++index) {
            char character = token_candidate[0];

            bool initial_state = !starts_with_digits &&
                                 !dot_after_start_digits &&
                                 !ends_with_digits_after_dot;
            if (initial_state && is_ascii_digit(character)) {
              starts_with_digits = true;
              continue;
            }

            bool state_after_first_digits = starts_with_digits &&
                                            !dot_after_start_digits &&
                                            !ends_with_digits_after_dot;
            if (state_after_first_digits && character == '.') {
              dot_after_start_digits = true;
              continue;
            }

            bool state_after_dot = starts_with_digits &&
                                   dot_after_start_digits &&
                                   !ends_with_digits_after_dot;
            if (state_after_dot && is_ascii_digit(character)) {
              ends_with_digits_after_dot = true;
              continue;
            }

            bool valid_float_being_expanded = starts_with_digits &&
                                              dot_after_start_digits &&
                                              ends_with_digits_after_dot;
            if (valid_float_being_expanded && is_ascii_digit(character)) {
              continue;
            }

            if (!is_ascii_digit(character)) {
              return TokenDefinitionPredicateResult::TokenInvalid;
            }
          }
          bool is_postfix_a_digit = everything_after.size() > 0 &&
                                    is_ascii_digit(everything_after[0]);

          if (starts_with_digits && dot_after_start_digits &&
              ends_with_digits_after_dot && !is_postfix_a_digit) {
            return TokenDefinitionPredicateResult::
                TokenIsValidAndShouldBeFlushed;
          }

          return TokenDefinitionPredicateResult::
              TokenIsValidButShouldBeExpanded;
        }}};

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
  case TokenKind::FloatLiteral:
    return "FloatLiteral";
  case TokenKind::Addition:
    return "Addition";
  case TokenKind::Assignment:
    return "Assignment";
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
  std::vector<Token> tokens_found = {};

  std::vector<char> chars_before_current_token = {};
  std::vector<char> chars_after_current_token = file_vector;

  std::vector<char> current_classification_candidate = {};
  std::vector<TokenKind> tokens_that_want_expanding = {};
  std::vector<TokenKind> tokens_that_want_flushing = {};

  for (size_t index = 0; index < file_vector.size(); ++index) {
    char current_character = file_vector[index];
    current_classification_candidate.push_back(current_character);

    size_t token_start_index =
        (index + 1) - current_classification_candidate.size();
    chars_before_current_token.assign(file_vector.begin(),
                                      file_vector.begin() + token_start_index);

    chars_after_current_token.assign(file_vector.begin() + index + 1,
                                     file_vector.end());
    tokens_that_want_expanding = {};
    tokens_that_want_flushing = {};

    for (auto token_definition : token_definitions) {
      TokenDefinitionPredicateResult candidate_predicate_result =
          token_definition.predicate(current_classification_candidate,
                                     chars_before_current_token,
                                     chars_after_current_token);

      if (candidate_predicate_result ==
          TokenDefinitionPredicateResult::TokenIsValidAndShouldBeFlushed) {
        tokens_that_want_flushing.push_back(token_definition.kind);
      }

      if (candidate_predicate_result ==
          TokenDefinitionPredicateResult::TokenIsValidButShouldBeExpanded) {
        tokens_that_want_expanding.push_back(token_definition.kind);
      }
    }

    // for(tokens_that_want_expanding)
    //
    if(tokens_that_want_expanding.size() > 0) {
      // traceback mechanism omfg
    }

    if (tokens_that_want_flushing.size() > 0 && tokens_that_want_expanding.size() == 0) {
      ASSERT_THAT("there cant be multiple full matches with single token",
                  tokens_that_want_flushing.size() == 1);
      std::string token_value(current_classification_candidate.begin(),
                              current_classification_candidate.end());

      tokens_found.push_back(
          Token{.kind = tokens_that_want_flushing[0], .value = token_value});

      current_classification_candidate = {};
      continue;
    }

    if (tokens_that_want_expanding.size() > 0)
      continue;

    bool nothing_seems_valid = tokens_that_want_expanding.size() == 0 &&
                               tokens_that_want_flushing.size() == 0;

    if (nothing_seems_valid) {
      bool this_is_invisible_delimiter = current_character == ' ' ||
                                         current_character == '\r' ||
                                         current_character == '\n';
      if (this_is_invisible_delimiter) {
        current_classification_candidate = {};
        continue;
      }

      printf("\ninvalid stuff encountered '%c' on index '%zu' \n",
             current_character, index);
      std::exit(EXIT_FAILURE);
    }
  }

  for (auto token : tokens_found) {
    printf("\n%s : '%s'", token_kind_to_string(token.kind).c_str(),
           token.value.c_str());
  }

  return 0;
};
