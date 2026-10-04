#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <format>
#include <fstream>
#include <ios>
#include <print>
#include <string>
#include <utility>
#include <vector>

using std::string;

void TODO(string description, bool condition_is_ok) {
  if (condition_is_ok)
    return;

  std::println(stderr, "EXIT_FAILURE [TODO]: {}", description);
  std::exit(EXIT_FAILURE);
}

void ASSERT_THAT(string description, bool when_false_causes_exit) {
  if (when_false_causes_exit == true)
    return;

  std::println(stderr, "EXIT_FAILURE [ASSERT]: {}", description);
  std::exit(EXIT_FAILURE);
}

// TODO: could use the std::variant thingy
enum class TokenKind {
  None = 0,
  Identifier,
  IntegerLiteral,
  EqualLiteral,
  StatementEnding
};

struct Token {
  TokenKind kind = TokenKind::None;
  // TODO: use string view or sth else to avoid heap allocations/reallocations -
  // all token values are already in the file buffer
  // TODO: could use an offset with a file name id e.g. start_index, end_index
  string value = {};
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

  std::unreachable();
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
// expand on something to the point you get to character that cannot be counted into current token
// then flush the thing
// then start from that as a new token?
//
// Where could there be a problem with this?
// well for instance you could have something like ascii-only identifiers
// then you have keyword that has non-ascii thing like LIST-ME (with - that isn't supported amongst identifiers)
//
// the edge case where it breaks would be a case where someone tries to use this as identifier because
// you then have [list] - identifier [-] unsupported/minus operator [me] identifier
// what role does the \n play?
// what role does the ; play?
// 
void expand_token(TokenKind kind, char new_character) {
  using namespace std::ranges;

  bool is_character_invalid =
      !contains(all_legal_characters, new_character);
  if (is_character_invalid) {
    std::println(stderr, "failure, invalid_character_found {}", character);
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
//      auto (new_classification, is_in_final_form_and_cannot_be_expanded, // kinda doesnt matter if final form because next character can be invalid expansion KEYWORDa some char hanging off
//      should_error, error_message)
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
// - labeling something as identifier and then modifying this to call it a keyword is goofy, maybe faster but coupled (assumes that the keyword and identifiers come from the same subset which doesn't have to be true)
//
// 

int main() {
  std::println("start");

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
  std::vector<Token> found_character_clusters = {};

  std::println("all_legal_characters: {}", all_legal_characters);

  string current_character_cluster = {};
  TokenKind current_cluster_classification = TokenKind::None;
  for (char character : file_vector) {

    bool this_is_a_new_cluster = current_character_cluster == std::string{};

    // SECTION: handle invalid character
    // bool is_character_invalid =
    //     !std::ranges::contains(all_legal_characters, character);
    // if (is_character_invalid) {
    //   std::println(stderr, "failure, invalid_character_found {}", character);
    //   std::exit(EXIT_FAILURE);
    // }

    // SECTION: handle statement ending operator
    bool
        new_unidentified_cluster_and_character_meets_statement_ending_criteria =
            this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::None &&
            character == ';';
    if (new_unidentified_cluster_and_character_meets_statement_ending_criteria) {
      current_character_cluster += character;
      current_cluster_classification = TokenKind::StatementEnding;
      continue;
    }

    bool
        ongoing_cluster_identified_as_statement_ending_and_current_character_is_space_or_newline =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::StatementEnding &&
            (std::ranges::contains(space, character) ||
             std::ranges::contains(new_lines, character));
    if (ongoing_cluster_identified_as_statement_ending_and_current_character_is_space_or_newline) {
      found_character_clusters.push_back(Token{
          .kind = current_cluster_classification,
          .value = current_character_cluster,
      });
      current_cluster_classification = TokenKind::None;
      current_character_cluster = {};
      continue;
    }

    bool
        ongoing_cluster_identified_as_statement_ending_and_current_character_is_also_a_statement_ending =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::StatementEnding &&
            character == ';';
    if (ongoing_cluster_identified_as_statement_ending_and_current_character_is_also_a_statement_ending) {
      found_character_clusters.push_back(Token{
          .kind = current_cluster_classification,
          .value = current_character_cluster,
      });
      current_cluster_classification = TokenKind::StatementEnding;
      current_character_cluster = {};
      current_character_cluster += character;
      continue;
    }

    bool
        ongoing_cluster_identified_as_something_different_than_statement_ending_or_none_and_current_character_is_statement_ending =
            !this_is_a_new_cluster &&
            current_cluster_classification != TokenKind::StatementEnding &&
            current_cluster_classification != TokenKind::None &&
            character == ';';
    if (ongoing_cluster_identified_as_something_different_than_statement_ending_or_none_and_current_character_is_statement_ending) {
      found_character_clusters.push_back(Token{
          .kind = current_cluster_classification,
          .value = current_character_cluster,
      });
      current_cluster_classification = TokenKind::StatementEnding;
      current_character_cluster = {};
      current_character_cluster += character;
      continue;
    }

    // SECTION: handle identifier (variable name)
    bool new_unidentified_cluster_and_character_meets_all_identifier_criteria =
        this_is_a_new_cluster &&
        current_cluster_classification == TokenKind::None &&
        std::ranges::contains(latin_letters, character);
    if (new_unidentified_cluster_and_character_meets_all_identifier_criteria) {
      current_cluster_classification = TokenKind::Identifier;
      current_character_cluster += character;
      continue;
    }

    bool
        ongoing_cluster_classified_as_identifier_and_current_character_meets_all_identifier_criteria =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::Identifier &&
            std::ranges::contains(latin_letters, character);
    if (ongoing_cluster_classified_as_identifier_and_current_character_meets_all_identifier_criteria) {
      current_character_cluster += character;
      continue;
    }

    bool
        ongoing_cluster_classified_as_identifier_and_current_character_is_space_or_new_line_or_statement_ending =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::Identifier &&
            (std::ranges::contains(space, character) ||
             std::ranges::contains(new_lines, character));

    if (ongoing_cluster_classified_as_identifier_and_current_character_is_space_or_new_line_or_statement_ending) {
      // TODO: VERIFY IF IDENTIFIER IS NOT A RESERVED KEYWORD TO BE CLASSIFIED
      // if(current_character_cluster == is a keyword )
      // although might be worth doing this somewhere else and variable naming
      // might have different set of rules than keywords, maybe doing this
      // earlier would make more sense e.g. variable names shouldnt have spaces
      // generally
      found_character_clusters.push_back(Token{
          .kind = current_cluster_classification,
          .value = current_character_cluster,
      });
      current_cluster_classification = TokenKind::None;
      current_character_cluster = {};
      continue;
    }

    bool
        ongoing_cluster_classified_as_identifier_and_current_character_does_not_meet_identifier_criteria_and_is_not_space =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::Identifier &&
            !std::ranges::contains(latin_letters, character) &&
            !std::ranges::contains(space, character);

    if (ongoing_cluster_classified_as_identifier_and_current_character_does_not_meet_identifier_criteria_and_is_not_space) {
      // TODO: VERIFY IF IDENTIFIER IS NOT A RESERVED KEYWORD TO BE CLASSIFIED
      // if(current_character_cluster == is a keyword )
      std::println(stderr,
                   "failure, invalid character found while classifying an "
                   "identifier {}[{}] <- invalid character",
                   current_character_cluster, character);
      std::exit(EXIT_FAILURE);
      continue;
    }

    // SECTION: handle uint64
    bool
        new_unidentified_cluster_and_character_meets_all_integer_literal_criteria =
            this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::None &&
            std::ranges::contains(digits, character);
    if (new_unidentified_cluster_and_character_meets_all_integer_literal_criteria) {
      current_character_cluster += character;
      current_cluster_classification = TokenKind::IntegerLiteral;
      continue;
    }

    bool
        ongoing_cluster_identified_as_integer_literal_and_character_meets_all_integer_literal_criteria =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::IntegerLiteral &&
            std::ranges::contains(digits, character);
    if (ongoing_cluster_identified_as_integer_literal_and_character_meets_all_integer_literal_criteria) {
      current_character_cluster += character;
      continue;
    }

    bool
        ongoing_cluster_identified_as_integer_literal_and_character_is_space_or_new_line =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::IntegerLiteral &&
            (std::ranges::contains(space, character) ||
             std::ranges::contains(new_lines, character));

    if (ongoing_cluster_identified_as_integer_literal_and_character_is_space_or_new_line) {
      found_character_clusters.push_back(Token{
          .kind = current_cluster_classification,
          .value = current_character_cluster,
      });
      current_cluster_classification = TokenKind::None;
      current_character_cluster = {};
      continue;
    }

    bool
        ongoing_cluster_identified_as_integer_literal_and_character_doesnt_meet_all_integer_literal_criteria =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::IntegerLiteral &&
            !std::ranges::contains(digits, character);
    if (ongoing_cluster_identified_as_integer_literal_and_character_doesnt_meet_all_integer_literal_criteria) {
      std::println(stderr,
                   "failure, invalid character found while classifying integer "
                   "literal {}[{}] <- invalid character",
                   current_character_cluster, character);
      std::exit(EXIT_FAILURE);
      continue;
    }

    // SECTION: handle assignment operator
    bool new_unidentified_cluster_and_character_meets_assignment_criteria =
        this_is_a_new_cluster &&
        current_cluster_classification == TokenKind::None && character == '=';
    if (new_unidentified_cluster_and_character_meets_assignment_criteria) {
      current_character_cluster += character;
      current_cluster_classification = TokenKind::EqualLiteral;
      continue;
    }

    bool
        ongoing_cluster_identified_as_assignment_and_character_is_space_or_newline =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::EqualLiteral &&
            (std::ranges::contains(space, character) ||
             std::ranges::contains(new_lines, character));
    if (ongoing_cluster_identified_as_assignment_and_character_is_space_or_newline) {
      found_character_clusters.push_back(Token{
          .kind = current_cluster_classification,
          .value = current_character_cluster,
      });
      current_cluster_classification = TokenKind::None;
      current_character_cluster = {};
      continue;
    }

    bool
        ongoing_cluster_identified_as_assignment_and_character_is_not_space_not_new_line_meaning_weird_continuation =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::EqualLiteral &&
            (!std::ranges::contains(space, character) &&
             !std::ranges::contains(new_lines, character));
    if (ongoing_cluster_identified_as_assignment_and_character_is_not_space_not_new_line_meaning_weird_continuation) {
      std::println(stderr,
                   "failure, invalid character found while evaluating "
                   "assignment operator {}[{}] <- invalid character",
                   current_character_cluster, character);
      std::exit(EXIT_FAILURE);
      continue;
    }
    // SECTION: handle empty space
    bool new_unidentified_cluster_and_current_character_is_space_or_new_line =
        this_is_a_new_cluster &&
        current_cluster_classification == TokenKind::None &&
        (std::ranges::contains(space, character) ||
         std::ranges::contains(new_lines, character));
    if (new_unidentified_cluster_and_current_character_is_space_or_new_line) {
      continue;
    }

    // SECTION: handle potentially missed edge cases
    bool we_didnt_get_here = false;
    std::println("DEBUG_STATE: \n - CHARACTER: {} \n - CLUSTER: {}", character,
                 current_character_cluster);
    ASSERT_THAT("no execution path should lead here. this means a state "
                "invariant of sorts was not predicted",
                we_didnt_get_here);
  }

  for (Token t : found_character_clusters) {
    std::println(stderr, "token: ['{}' '{}']", t.value,
                 token_kind_to_string(t.kind));
  }

  // SECTION: grammar interpretation
  std::vector<Token> current_statement = {};
  for (Token t : found_character_clusters) {

    bool this_is_a_new_statement = current_statement.size();

    if (TokenKind::IntegerLiteral == t.kind) {
      uint64_t out_parse_int_result = 0;
      bool this_is_uint64 = try_parse_uint_64(t.value, out_parse_int_result);

      if (!this_is_uint64) {
        std::println(
            stderr, "non-uint64 values are not supported for integer literals");
        std::exit(EXIT_FAILURE);
      };

      if (this_is_uint64) {
      }
    };

    // if(this_is_a_new_statement)
  };

  return 0;
};
