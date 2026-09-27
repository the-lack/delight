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
  Assignment,
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
  case TokenKind::Assignment:
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

  std::vector<char> all_legal_characters;
  for (const auto &vec : {latin_letters, digits, new_lines, operators, space,
                          special_characters, statement_end}) {
    std::ranges::copy(vec, std::back_inserter(all_legal_characters));
  }

  // std::vector<>

  std::vector<Token> found_character_clusters = {};

  std::println("all_legal_characters: {}", all_legal_characters);

  string current_character_cluster = {};
  TokenKind current_cluster_classification = TokenKind::None;
  for (char character : file_vector) {

    bool this_is_a_new_cluster = current_character_cluster == std::string{};

    // SECTION: handle invalid character
    bool is_character_invalid =
        !std::ranges::contains(all_legal_characters, character);
    if (is_character_invalid) {
      std::println(stderr, "failure, invalid_character_found {}", character);
      std::exit(EXIT_FAILURE);
    }

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
        .kind =  current_cluster_classification,
        .value =  current_character_cluster,
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
        .kind =  current_cluster_classification,
        .value =  current_character_cluster,
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
        .kind =  current_cluster_classification,
        .value =  current_character_cluster,
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
        .kind =  current_cluster_classification,
        .value =  current_character_cluster,
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
        .kind =  current_cluster_classification,
        .value =  current_character_cluster,
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
      current_cluster_classification = TokenKind::Assignment;
      continue;
    }

    bool
        ongoing_cluster_identified_as_assignment_and_character_is_space_or_newline =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::Assignment &&
            (std::ranges::contains(space, character) ||
             std::ranges::contains(new_lines, character));
    if (ongoing_cluster_identified_as_assignment_and_character_is_space_or_newline) {
      found_character_clusters.push_back(Token{
        .kind =  current_cluster_classification,
        .value =  current_character_cluster,
      });      
      current_cluster_classification = TokenKind::None;
      current_character_cluster = {};
      continue;
    }

    bool
        ongoing_cluster_identified_as_assignment_and_character_is_not_space_not_new_line_meaning_weird_continuation =
            !this_is_a_new_cluster &&
            current_cluster_classification == TokenKind::Assignment &&
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

  for(Token t: found_character_clusters ){
    std::println(stderr, "token: ['{}' '{}']", t.value, token_kind_to_string(t.kind));
  }
  

  return 0;
};
