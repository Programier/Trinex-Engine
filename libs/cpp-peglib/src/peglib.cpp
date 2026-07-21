#include "peglib.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace peg {

// Keyword-guarded identifier data, heap-allocated only for matching Sequences.
// Avoids bloating all Sequence objects with bitsets and keyword sets.
struct KeywordGuardData {
  std::bitset<256> identifier_first;        // first char of identifier
  std::bitset<256> identifier_rest;         // subsequent chars of identifier
  std::vector<std::string> exact_keywords;  // single-word keywords (lowercase)
  std::vector<std::string> prefix_keywords; // first word of compound keywords
  size_t min_keyword_len = 0;
  size_t max_keyword_len = 0;

  static bool matches_any(const std::vector<std::string> &keywords,
                          std::string_view input) {
    return std::any_of(keywords.begin(), keywords.end(),
                       [&](const auto &kw) { return kw == input; });
  }
};

Ope::Visitor::~Visitor() {}
void Ope::Visitor::visit(Sequence &) {}
void Ope::Visitor::visit(PrioritizedChoice &) {}
void Ope::Visitor::visit(Repetition &) {}
void Ope::Visitor::visit(AndPredicate &) {}
void Ope::Visitor::visit(NotPredicate &) {}
void Ope::Visitor::visit(Dictionary &) {}
void Ope::Visitor::visit(LiteralString &) {}
void Ope::Visitor::visit(CharacterClass &) {}
void Ope::Visitor::visit(Character &) {}
void Ope::Visitor::visit(AnyCharacter &) {}
void Ope::Visitor::visit(CaptureScope &) {}
void Ope::Visitor::visit(Capture &) {}
void Ope::Visitor::visit(TokenBoundary &) {}
void Ope::Visitor::visit(Ignore &) {}
void Ope::Visitor::visit(User &) {}
void Ope::Visitor::visit(WeakHolder &) {}
void Ope::Visitor::visit(Holder &) {}
void Ope::Visitor::visit(Reference &) {}
void Ope::Visitor::visit(Whitespace &) {}
void Ope::Visitor::visit(BackReference &) {}
void Ope::Visitor::visit(PrecedenceClimbing &) {}
void Ope::Visitor::visit(Recovery &) {}
void Ope::Visitor::visit(Cut &) {}

const char *const WHITESPACE_DEFINITION_NAME = "%whitespace";
const char *const WORD_DEFINITION_NAME = "%word";
const char *const RECOVER_DEFINITION_NAME = "%recover";

struct TraversalVisitor : public Ope::Visitor {
  using Ope::Visitor::visit;
  void visit(Sequence &ope) override {
    for (auto &op : ope.opes_) {
      op->accept(*this);
    }
  }
  void visit(PrioritizedChoice &ope) override {
    for (auto &op : ope.opes_) {
      op->accept(*this);
    }
  }
  void visit(Repetition &ope) override { ope.ope_->accept(*this); }
  void visit(AndPredicate &ope) override { ope.ope_->accept(*this); }
  void visit(NotPredicate &ope) override { ope.ope_->accept(*this); }
  void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
  void visit(Capture &ope) override { ope.ope_->accept(*this); }
  void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
  void visit(Ignore &ope) override { ope.ope_->accept(*this); }
  void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
  void visit(Holder &ope) override { ope.ope_->accept(*this); }
  void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
  void visit(Recovery &ope) override { ope.ope_->accept(*this); }
  void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
};

struct TraceOpeName : public Ope::Visitor {
  using Ope::Visitor::visit;

  void visit(Sequence &) override { name_ = "Sequence"; }
  void visit(PrioritizedChoice &) override { name_ = "PrioritizedChoice"; }
  void visit(Repetition &) override { name_ = "Repetition"; }
  void visit(AndPredicate &) override { name_ = "AndPredicate"; }
  void visit(NotPredicate &) override { name_ = "NotPredicate"; }
  void visit(Dictionary &) override { name_ = "Dictionary"; }
  void visit(LiteralString &) override { name_ = "LiteralString"; }
  void visit(CharacterClass &) override { name_ = "CharacterClass"; }
  void visit(Character &) override { name_ = "Character"; }
  void visit(AnyCharacter &) override { name_ = "AnyCharacter"; }
  void visit(CaptureScope &) override { name_ = "CaptureScope"; }
  void visit(Capture &) override { name_ = "Capture"; }
  void visit(TokenBoundary &) override { name_ = "TokenBoundary"; }
  void visit(Ignore &) override { name_ = "Ignore"; }
  void visit(User &) override { name_ = "User"; }
  void visit(WeakHolder &) override { name_ = "WeakHolder"; }
  void visit(Holder &ope) override { name_ = ope.trace_name().data(); }
  void visit(Reference &) override { name_ = "Reference"; }
  void visit(Whitespace &) override { name_ = "Whitespace"; }
  void visit(BackReference &) override { name_ = "BackReference"; }
  void visit(PrecedenceClimbing &) override { name_ = "PrecedenceClimbing"; }
  void visit(Recovery &) override { name_ = "Recovery"; }
  void visit(Cut &) override { name_ = "Cut"; }

  static std::string get(Ope &ope) {
    TraceOpeName vis;
    ope.accept(vis);
    return vis.name_;
  }

private:
  const char *name_ = nullptr;
};

struct AssignIDToDefinition : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(Holder &ope) override;
  void visit(Reference &ope) override;
  void visit(PrecedenceClimbing &ope) override;

  std::unordered_map<void *, size_t> ids;
};

struct IsLiteralToken : public Ope::Visitor {
  using Ope::Visitor::visit;

  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      if (!IsLiteralToken::check(*op)) { return; }
    }
    result_ = true;
  }

  void visit(Dictionary &) override { result_ = true; }
  void visit(LiteralString &) override { result_ = true; }

  static bool check(Ope &ope) {
    IsLiteralToken vis;
    ope.accept(vis);
    return vis.result_;
  }

private:
  bool result_ = false;
};

struct TokenChecker : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(TokenBoundary &) override { has_token_boundary_ = true; }
  void visit(AndPredicate &) override {}
  void visit(NotPredicate &) override {}
  void visit(WeakHolder &) override { has_rule_ = true; }
  void visit(Reference &ope) override;

  static bool is_token(Ope &ope) {
    if (IsLiteralToken::check(ope)) { return true; }

    TokenChecker vis;
    ope.accept(vis);
    return vis.has_token_boundary_ || !vis.has_rule_;
  }

private:
  bool has_token_boundary_ = false;
  bool has_rule_ = false;
};

struct FindLiteralToken : public Ope::Visitor {
  using Ope::Visitor::visit;

  void visit(LiteralString &ope) override { token_ = ope.lit_.data(); }
  void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
  void visit(Ignore &ope) override { ope.ope_->accept(*this); }
  void visit(Reference &ope) override;
  void visit(Recovery &ope) override { ope.ope_->accept(*this); }

  static const char *token(Ope &ope) {
    FindLiteralToken vis;
    ope.accept(vis);
    return vis.token_;
  }

private:
  const char *token_ = nullptr;
};

struct DetectLeftRecursion : public TraversalVisitor {
  using TraversalVisitor::visit;

  DetectLeftRecursion(const std::string &name) : name_(name) {}

  void visit(Sequence &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (done_) {
        break;
      } else if (error_s) {
        done_ = true;
        break;
      }
    }
  }
  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (error_s) {
        done_ = true;
        break;
      }
    }
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    done_ = ope.min_ > 0;
  }
  void visit(AndPredicate &ope) override {
    ope.ope_->accept(*this);
    done_ = false;
  }
  void visit(NotPredicate &ope) override {
    ope.ope_->accept(*this);
    done_ = false;
  }
  void visit(Dictionary &) override { done_ = true; }
  void visit(LiteralString &ope) override { done_ = !ope.lit_.empty(); }
  void visit(CharacterClass &) override { done_ = true; }
  void visit(Character &) override { done_ = true; }
  void visit(AnyCharacter &) override { done_ = true; }
  void visit(User &) override { done_ = true; }
  void visit(Reference &ope) override;
  void visit(BackReference &) override { done_ = true; }
  void visit(Cut &) override { done_ = true; }

  const char *error_s = nullptr;

  std::shared_ptr<Ope> resolve_macro_arg(size_t iarg) const;

private:
  std::string name_;
  std::unordered_set<std::string> refs_;
  bool done_ = false;
  std::vector<const std::vector<std::shared_ptr<Ope>> *> macro_args_stack_;
};

struct ComputeCanBeEmpty : public TraversalVisitor {
  using TraversalVisitor::visit;

  bool result = false;

  void visit(Sequence &ope) override {
    result = std::all_of(ope.opes_.begin(), ope.opes_.end(), [](auto &op) {
      ComputeCanBeEmpty vis;
      op->accept(vis);
      return vis.result;
    });
  }
  void visit(PrioritizedChoice &ope) override {
    result = std::any_of(ope.opes_.begin(), ope.opes_.end(), [](auto &op) {
      ComputeCanBeEmpty vis;
      op->accept(vis);
      return vis.result;
    });
  }
  void visit(Repetition &ope) override { result = ope.min_ == 0; }
  void visit(AndPredicate &) override { result = true; }
  void visit(NotPredicate &) override { result = true; }
  void visit(Dictionary &) override { result = false; }
  void visit(LiteralString &ope) override { result = ope.lit_.empty(); }
  void visit(CharacterClass &) override { result = false; }
  void visit(Character &) override { result = false; }
  void visit(AnyCharacter &) override { result = false; }
  void visit(User &) override { result = false; }
  void visit(Reference &ope) override;
  void visit(BackReference &) override { result = false; }
  void visit(Cut &) override { result = false; }
};

struct HasEmptyElement : public TraversalVisitor {
  using TraversalVisitor::visit;

  HasEmptyElement(std::vector<std::pair<const char *, std::string>> &refs,
                  std::unordered_map<std::string, bool> &has_error_cache)
      : refs_(refs), has_error_cache_(has_error_cache) {}

  void visit(Sequence &ope) override;
  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (is_empty) { return; }
    }
  }
  void visit(Repetition &ope) override {
    if (ope.min_ == 0) {
      set_error();
    } else {
      ope.ope_->accept(*this);
    }
  }
  void visit(AndPredicate &) override { set_error(); }
  void visit(NotPredicate &) override { set_error(); }
  void visit(LiteralString &ope) override {
    if (ope.lit_.empty()) { set_error(); }
  }
  void visit(Reference &ope) override;

  bool is_empty = false;
  const char *error_s = nullptr;
  std::string error_name;

private:
  void set_error() {
    is_empty = true;
    tie(error_s, error_name) = refs_.back();
  }
  std::vector<std::pair<const char *, std::string>> &refs_;
  std::unordered_map<std::string, bool> &has_error_cache_;
};

struct DetectInfiniteLoop : public TraversalVisitor {
  using TraversalVisitor::visit;

  DetectInfiniteLoop(const char *s, const std::string &name,
                     std::vector<std::pair<const char *, std::string>> &refs,
                     std::unordered_map<std::string, bool> &has_error_cache)
      : refs_(refs), has_error_cache_(has_error_cache) {
    refs_.emplace_back(s, name);
  }

  DetectInfiniteLoop(std::vector<std::pair<const char *, std::string>> &refs,
                     std::unordered_map<std::string, bool> &has_error_cache)
      : refs_(refs), has_error_cache_(has_error_cache) {}

  void visit(Sequence &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (has_error) { return; }
    }
  }
  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (has_error) { return; }
    }
  }
  void visit(Repetition &ope) override {
    if (ope.max_ == std::numeric_limits<size_t>::max()) {
      HasEmptyElement vis(refs_, has_error_cache_);
      ope.ope_->accept(vis);
      if (vis.is_empty) {
        has_error = true;
        error_s = vis.error_s;
        error_name = vis.error_name;
      }
    } else {
      ope.ope_->accept(*this);
    }
  }
  void visit(Reference &ope) override;

  bool has_error = false;
  const char *error_s = nullptr;
  std::string error_name;

private:
  std::vector<std::pair<const char *, std::string>> &refs_;
  std::unordered_map<std::string, bool> &has_error_cache_;
};

struct ReferenceChecker : public TraversalVisitor {
  using TraversalVisitor::visit;

  ReferenceChecker(const Grammar &grammar,
                   const std::vector<std::string> &params)
      : grammar_(grammar), params_(params) {}

  void visit(Reference &ope) override;

  std::unordered_map<std::string, const char *> error_s;
  std::unordered_map<std::string, std::string> error_message;
  std::unordered_set<std::string> referenced;

private:
  const Grammar &grammar_;
  const std::vector<std::string> &params_;
};

struct LinkReferences : public TraversalVisitor {
  using TraversalVisitor::visit;

  LinkReferences(Grammar &grammar, const std::vector<std::string> &params)
      : grammar_(grammar), params_(params) {}

  void visit(Reference &ope) override;

private:
  Grammar &grammar_;
  const std::vector<std::string> &params_;
};

struct FindReference : public Ope::Visitor {
  using Ope::Visitor::visit;

  FindReference(const std::vector<std::shared_ptr<Ope>> &args,
                const std::vector<std::string> &params)
      : args_(args), params_(params) {}

  void visit(Sequence &ope) override {
    std::vector<std::shared_ptr<Ope>> opes;
    for (const auto &o : ope.opes_) {
      o->accept(*this);
      opes.emplace_back(std::move(found_ope));
    }
    found_ope = std::make_shared<Sequence>(opes);
  }
  void visit(PrioritizedChoice &ope) override {
    std::vector<std::shared_ptr<Ope>> opes;
    for (const auto &o : ope.opes_) {
      o->accept(*this);
      opes.emplace_back(std::move(found_ope));
    }
    found_ope = std::make_shared<PrioritizedChoice>(opes);
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    found_ope = rep(found_ope, ope.min_, ope.max_);
  }
  void visit(AndPredicate &ope) override {
    ope.ope_->accept(*this);
    found_ope = apd(found_ope);
  }
  void visit(NotPredicate &ope) override {
    ope.ope_->accept(*this);
    found_ope = npd(found_ope);
  }
  void visit(Dictionary &ope) override { found_ope = ope.shared_from_this(); }
  void visit(LiteralString &ope) override {
    found_ope = ope.shared_from_this();
  }
  void visit(CharacterClass &ope) override {
    found_ope = ope.shared_from_this();
  }
  void visit(Character &ope) override { found_ope = ope.shared_from_this(); }
  void visit(AnyCharacter &ope) override { found_ope = ope.shared_from_this(); }
  void visit(CaptureScope &ope) override {
    ope.ope_->accept(*this);
    found_ope = csc(found_ope);
  }
  void visit(Capture &ope) override {
    ope.ope_->accept(*this);
    found_ope = cap(found_ope, ope.match_action_);
  }
  void visit(TokenBoundary &ope) override {
    ope.ope_->accept(*this);
    found_ope = tok(found_ope);
  }
  void visit(Ignore &ope) override {
    ope.ope_->accept(*this);
    found_ope = ign(found_ope);
  }
  void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
  void visit(Holder &ope) override { ope.ope_->accept(*this); }
  void visit(Reference &ope) override;
  void visit(Whitespace &ope) override {
    ope.ope_->accept(*this);
    found_ope = wsp(found_ope);
  }
  void visit(PrecedenceClimbing &ope) override {
    ope.atom_->accept(*this);
    found_ope = csc(found_ope);
  }
  void visit(Recovery &ope) override {
    ope.ope_->accept(*this);
    found_ope = rec(found_ope);
  }
  void visit(Cut &ope) override { found_ope = ope.shared_from_this(); }

  std::shared_ptr<Ope> found_ope;

private:
  const std::vector<std::shared_ptr<Ope>> &args_;
  const std::vector<std::string> &params_;
};

/*
 * First-Set computation
 */
struct ComputeFirstSet : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(Sequence &ope) override {
    for (const auto &op : ope.opes_) {
      FirstSet element_fs;
      auto save = result_;
      result_ = FirstSet{};
      op->accept(*this);
      element_fs = result_;
      result_ = save;
      result_.chars |= element_fs.chars;
      if (element_fs.any_char) { result_.any_char = true; }
      if (!result_.first_literal) {
        result_.first_literal = element_fs.first_literal;
      }
      if (!result_.first_rule) { result_.first_rule = element_fs.first_rule; }
      if (!element_fs.can_be_empty) { return; }
      // This element can be empty, continue to next
    }
    result_.can_be_empty = true;
  }
  void visit(PrioritizedChoice &ope) override {
    auto save = result_;
    for (const auto &op : ope.opes_) {
      result_ = FirstSet{};
      op->accept(*this);
      save.merge(result_);
    }
    result_ = save;
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    if (ope.min_ == 0) { result_.can_be_empty = true; }
  }
  void visit(AndPredicate &) override { result_.can_be_empty = true; }
  void visit(NotPredicate &) override { result_.can_be_empty = true; }
  void visit(Dictionary &ope) override {
    for (const auto &[key, info] : ope.trie_.dic_) {
      if (!key.empty()) {
        auto ch = static_cast<unsigned char>(key[0]);
        result_.chars.set(ch);
        if (ope.trie_.ignore_case_) {
          result_.chars.set(static_cast<unsigned char>(std::toupper(ch)));
          result_.chars.set(static_cast<unsigned char>(std::tolower(ch)));
        }
      }
    }
  }
  void visit(LiteralString &ope) override {
    if (ope.lit_.empty()) {
      result_.can_be_empty = true;
    } else {
      auto ch = static_cast<unsigned char>(ope.lit_[0]);
      result_.chars.set(ch);
      if (ope.ignore_case_) {
        result_.chars.set(static_cast<unsigned char>(std::toupper(ch)));
        result_.chars.set(static_cast<unsigned char>(std::tolower(ch)));
      }
      if (!result_.first_literal) { result_.first_literal = ope.lit_.c_str(); }
    }
  }
  void visit(CharacterClass &ope) override {
    for (const auto &range : ope.ranges_) {
      auto cp1 = range.first;
      auto cp2 = range.second;
      if (cp1 > 0x7F || cp2 > 0x7F) {
        // Non-ASCII range: conservative fallback
        result_.any_char = true;
        return;
      }
      for (auto cp = cp1; cp <= cp2; cp++) {
        auto ch = static_cast<unsigned char>(cp);
        result_.chars.set(ch);
        if (ope.ignore_case_) {
          result_.chars.set(static_cast<unsigned char>(std::toupper(ch)));
          result_.chars.set(static_cast<unsigned char>(std::tolower(ch)));
        }
      }
    }
    if (ope.negated_) {
      result_.chars.flip();
      result_.any_char = true; // negated class can match non-ASCII
    }
  }
  void visit(Character &ope) override {
    if (ope.ch_ > 0x7F) {
      result_.any_char = true;
    } else {
      result_.chars.set(static_cast<unsigned char>(ope.ch_));
    }
  }
  void visit(AnyCharacter &) override { result_.any_char = true; }
  void visit(User &) override { result_.any_char = true; }
  void visit(Reference &ope) override;
  void visit(BackReference &) override { result_.any_char = true; }
  void visit(Cut &) override { result_.can_be_empty = true; }

  // Per-rule cache shared across a SetupFirstSets traversal. Without it,
  // every alternative of every PrioritizedChoice re-walks referenced
  // rules — O(refs^depth) work for grammars with many cross-references.
  // Only cycle-free rule computations are cached; results computed under
  // a cycle (left recursion) would be incomplete and unsafe to reuse from
  // a different call context.
  using FirstSetCache = std::unordered_map<const Definition *, FirstSet>;

  explicit ComputeFirstSet(FirstSetCache &cache) : cache_(cache) {}

  FirstSet result_;

private:
  FirstSetCache &cache_;
  std::unordered_set<const Definition *> refs_;
  size_t cycle_count_ = 0;
};

struct SetupFirstSets : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(Sequence &ope) override;
  void setup_keyword_guarded_identifier(Sequence &ope);

  void visit(PrioritizedChoice &ope) override {
    ope.first_sets_.clear();
    ope.first_sets_.reserve(ope.opes_.size());
    for (const auto &op : ope.opes_) {
      ComputeFirstSet cfs(first_set_cache_);
      op->accept(cfs);
      ope.first_sets_.push_back(cfs.result_);
    }
    for (const auto &op : ope.opes_) {
      op->accept(*this);
    }
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    // ISpan optimization: detect Repetition + ASCII CharacterClass
    auto cc = dynamic_cast<CharacterClass *>(ope.ope_.get());
    if (cc && cc->is_ascii_only()) { ope.span_bitset_ = &cc->ascii_bitset(); }
  }
  void visit(Reference &ope) override;
  void visit(Holder &ope) override;

private:
  ComputeFirstSet::FirstSetCache first_set_cache_;
  std::unordered_set<const Definition *> visited_rules_;
};


SemanticValues::SemanticValues(Context *c) : c_(c) {}

std::string_view SemanticValues::sv() const { return sv_; }

const std::string &SemanticValues::name() const { return name_; }

size_t SemanticValues::choice_count() const { return choice_count_; }

size_t SemanticValues::choice() const { return choice_; }

std::string_view SemanticValues::token(size_t id) const {
    if (tokens.empty()) { return sv_; }
    assert(id < tokens.size());
    return tokens[id];
  }

std::string SemanticValues::token_to_string(size_t id) const {
    return std::string(token(id));
  }

Action::operator bool() const { return bool(fn_); }

std::any Action::operator()(SemanticValues &vs, std::any &dt,
                      const std::any &predicate_data) const {
    return fn_(vs, dt, predicate_data);
  }

Predicate::operator bool() const { return bool(fn_); }

bool Predicate::operator()(const SemanticValues &vs, const std::any &dt,
                  std::string &msg, std::any &predicate_data) const {
    return fn_(vs, dt, msg, predicate_data);
  }

void FirstSet::merge(const FirstSet &other) {
    chars |= other.chars;
    if (other.can_be_empty) { can_be_empty = true; }
    if (other.any_char) { any_char = true; }
    // Note: first_literal/first_rule are NOT merged — per-alternative
  }

Reference::Reference(const Grammar &grammar, const std::string &name, const char *s,
            bool is_macro, const std::vector<std::shared_ptr<Ope>> &args)
      : grammar_(grammar), name_(name), s_(s), is_macro_(is_macro), args_(args),
        rule_(nullptr), iarg_(0) {}

#if defined(__cpp_lib_char8_t)
Definition::Result Definition::parse(const char8_t *s, size_t n, const char *path,
               Log log) const {
    return parse(reinterpret_cast<const char *>(s), n, path, log);
  }

Definition::Result Definition::parse(const char8_t *s, const char *path,
               Log log) const {
    return parse(reinterpret_cast<const char *>(s), path, log);
  }

Definition::Result Definition::parse(const char8_t *s, size_t n, std::any &dt,
               const char *path, Log log) const {
    return parse(reinterpret_cast<const char *>(s), n, dt, path, log);
  }

Definition::Result Definition::parse(const char8_t *s, std::any &dt, const char *path,
               Log log) const {
    return parse(reinterpret_cast<const char *>(s), dt, path, log);
  }
#endif

/*
 * Core class methods
 */

Trie::Trie(const std::vector<std::string> &items, bool ignore_case)
      : ignore_case_(ignore_case), items_count_(items.size()) {
    size_t id = 0;
    for (const auto &item : items) {
      const auto &s = ignore_case ? to_lower(item) : item;
      if (item.size() > max_len_) { max_len_ = item.size(); }
      for (size_t len = 1; len <= item.size(); len++) {
        auto last = len == item.size();
        std::string_view sv(s.data(), len);
        auto it = dic_.find(sv);
        if (it == dic_.end()) {
          dic_.emplace(sv, Info{last, last, id});
        } else if (last) {
          it->second.match = true;
        } else {
          it->second.done = false;
        }
      }
      id++;
    }
  }

size_t Trie::match(const char *text, size_t text_len, size_t &id) const {
    auto limit = std::min(text_len, max_len_);
    std::string lower_text;
    if (ignore_case_) {
      lower_text = to_lower(std::string(text, limit));
      text = lower_text.data();
    }

    size_t match_len = 0;
    auto done = false;
    size_t len = 1;
    while (!done && len <= limit) {
      std::string_view sv(text, len);
      auto it = dic_.find(sv);
      if (it == dic_.end()) {
        done = true;
      } else {
        if (it->second.match) {
          match_len = len;
          id = it->second.id;
        }
        if (it->second.done) { done = true; }
      }
      len += 1;
    }
    return match_len;
  }

size_t Trie::size() const { return dic_.size(); }

size_t Trie::items_count() const { return items_count_; }

void ErrorInfo::clear() {
    error_pos = nullptr;
    expected_tokens.clear();
    message_pos = nullptr;
    message.clear();
  }

void ErrorInfo::add(const char *error_literal, const Definition *error_rule) {
    for (const auto &[t, r] : expected_tokens) {
      if (t == error_literal && r == error_rule) { return; }
    }
    expected_tokens.emplace_back(error_literal, error_rule);
  }

void ErrorInfo::output_log(const Log &log, const char *s, size_t n) {
    output_log(log, nullptr, s, n);
  }

int ErrorInfo::cast_char(char c) const { return static_cast<unsigned char>(c); }

std::string ErrorInfo::heuristic_error_token(const char *s, size_t n,
                                    const char *pos) const {
    auto len = n - std::distance(s, pos);
    if (len) {
      size_t i = 0;
      auto c = cast_char(pos[i++]);
      if (!std::ispunct(c) && !std::isspace(c)) {
        while (i < len && !std::ispunct(cast_char(pos[i])) &&
               !std::isspace(cast_char(pos[i]))) {
          i++;
        }
      }

      size_t count = CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT;
      size_t j = 0;
      while (count > 0 && j < i) {
        j += codepoint_length(&pos[j], i - j);
        count--;
      }

      return escape_characters(pos, j);
    }
    return std::string();
  }

std::string ErrorInfo::replace_all(std::string str, const std::string &from,
                          const std::string &to) const {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
      str.replace(pos, from.length(), to);
      pos += to.length();
    }
    return str;
  }

PackratCache::PackratCache(size_t expected_entries) {
    while (initial_capacity_ < expected_entries) {
      initial_capacity_ *= 2;
    }
  }

bool PackratCache::find(size_t key, size_t &len, std::any &val) const {
    if (slots_.empty()) { return false; }
    auto mask = slots_.size() - 1;
    auto i = mix(key) & mask;
    while (true) {
      auto &slot = slots_[i];
      if (slot.key == key) {
        len = slot.len;
        if (!vals_.empty()) {
          val = vals_[i];
        } else {
          val.reset();
        }
        return true;
      }
      if (slot.key == kEmpty) { return false; }
      i = (i + 1) & mask;
    }
  }

void PackratCache::insert_or_assign(size_t key, size_t len, const std::any &val) {
    if (slots_.empty() || (used_ + 1) * 4 > slots_.size() * 3) { grow(); }
    auto mask = slots_.size() - 1;
    auto i = mix(key) & mask;
    auto insert_pos = kEmpty;
    while (true) {
      auto &slot = slots_[i];
      if (slot.key == key) {
        insert_pos = i;
        break;
      }
      if (slot.key == kTombstone) {
        if (insert_pos == kEmpty) { insert_pos = i; }
      } else if (slot.key == kEmpty) {
        if (insert_pos == kEmpty) { insert_pos = i; }
        if (slots_[insert_pos].key == kEmpty) { used_++; }
        break;
      }
      i = (i + 1) & mask;
    }
    auto &dest = slots_[insert_pos];
    dest.key = key;
    dest.len = len;
    if (val.has_value()) {
      if (vals_.empty()) { vals_.resize(slots_.size()); }
      vals_[insert_pos] = val;
    } else if (!vals_.empty()) {
      vals_[insert_pos].reset();
    }
  }

void PackratCache::erase(size_t key) {
    if (slots_.empty()) { return; }
    auto mask = slots_.size() - 1;
    auto i = mix(key) & mask;
    while (true) {
      auto &slot = slots_[i];
      if (slot.key == key) {
        slot.key = kTombstone;
        if (!vals_.empty()) { vals_[i].reset(); }
        return;
      }
      if (slot.key == kEmpty) { return; }
      i = (i + 1) & mask;
    }
  }

size_t PackratCache::mix(size_t key) {
    // Mix in 64 bits so `h >> 32` stays well-defined where size_t is 32-bit
    // (wasm32); on 64-bit targets this is bit-identical to the size_t mix.
    auto h = static_cast<uint64_t>(key) * 0x9E3779B97F4A7C15ull;
    return static_cast<size_t>(h ^ (h >> 32));
  }

void PackratCache::grow() {
    auto new_cap = slots_.empty() ? initial_capacity_ : slots_.size() * 2;
    std::vector<Slot> old_slots = std::move(slots_);
    std::vector<std::any> old_vals = std::move(vals_);
    slots_.assign(new_cap, Slot{});
    if (!old_vals.empty()) { vals_.assign(new_cap, std::any()); }
    used_ = 0;
    auto mask = new_cap - 1;
    for (size_t j = 0; j < old_slots.size(); j++) {
      auto &slot = old_slots[j];
      if (slot.key == kEmpty || slot.key == kTombstone) { continue; }
      auto i = mix(slot.key) & mask;
      while (slots_[i].key != kEmpty) {
        i = (i + 1) & mask;
      }
      slots_[i] = slot;
      if (!old_vals.empty()) { vals_[i] = std::move(old_vals[j]); }
      used_++;
    }
  }

int32_t Context::cache_slot(size_t def_id) const {
    if (!packrat_index) { return static_cast<int32_t>(def_id); }
    return def_id < packrat_index->size() ? (*packrat_index)[def_id] : -1;
  }

void Context::clear_packrat_cache(const char *pos, size_t def_id) {
    if (!enablePackratParsing) { return; }
    auto slot = cache_slot(def_id);
    if (slot < 0) { return; }
    auto col = static_cast<size_t>(pos - s);
    auto idx = packrat_cached_count * col + static_cast<size_t>(slot);
    if (idx < cache_registered.size()) {
      cache_registered[idx] = false;
      cache_success[idx] = false;
    }
    cache_values.erase(idx);
  }

void Context::write_packrat_cache(const char *pos, size_t def_id, size_t len,
                           const std::any &val) {
    if (!enablePackratParsing) { return; }
    auto slot = cache_slot(def_id);
    if (slot < 0) { return; }
    auto col = pos - s;
    auto idx = packrat_cached_count * static_cast<size_t>(col) +
               static_cast<size_t>(slot);
    if (idx >= cache_registered.size()) { return; }
    cache_registered[idx] = true;
    cache_success[idx] = true;
    cache_values.insert_or_assign(idx, len, val);
  }

Context::Context(const char *path, const char *s, size_t l, size_t def_count,
          std::shared_ptr<Ope> whitespaceOpe, std::shared_ptr<Ope> wordOpe,
          bool enablePackratParsing, TracerEnter tracer_enter,
          TracerLeave tracer_leave, std::any trace_data, bool verbose_trace,
          Log log, ErrorReporter error_reporter,
          const std::vector<int32_t> *packrat_index,
          size_t packrat_cached_count)
      : path(path), s(s), l(l), whitespaceOpe(whitespaceOpe), wordOpe(wordOpe),
        def_count(def_count), enablePackratParsing(enablePackratParsing),
        packrat_index(packrat_index),
        packrat_cached_count(packrat_index ? packrat_cached_count : def_count),
        cache_registered(
            enablePackratParsing ? this->packrat_cached_count * (l + 1) : 0),
        cache_success(
            enablePackratParsing ? this->packrat_cached_count * (l + 1) : 0),
        active_pos(enablePackratParsing ? def_count : 0, nullptr),
        cache_values(enablePackratParsing ? (packrat_index ? l / 8 + 16 : l / 2)
                                          : 0),
        tracer_enter(tracer_enter), tracer_leave(tracer_leave),
        has_tracer(tracer_enter && tracer_leave), trace_data(trace_data),
        verbose_trace(verbose_trace), log(log), error_reporter(error_reporter) {

    for (size_t i = 0; i < 256; i++) {
      tolower_table[i] =
          static_cast<unsigned char>(std::tolower(static_cast<int>(i)));
    }

    push_args({});
  }

Context::~Context() {
    assert(!value_stack_size);
    assert(cut_stack.empty());
  }

SemanticValues &Context::push_semantic_values_scope() {
    assert(value_stack_size <= value_stack.size());
    if (value_stack_size == value_stack.size()) {
      value_stack.emplace_back(std::make_unique<SemanticValues>(this));
    } else {
      auto &vs = *value_stack[value_stack_size];
      if (!vs.empty()) {
        vs.clear();
        if (!vs.tags.empty()) { vs.tags.clear(); }
      }
      vs.sv_ = std::string_view();
      vs.choice_count_ = 0;
      vs.choice_ = 0;
      if (!vs.tokens.empty()) { vs.tokens.clear(); }
    }

    auto &vs = *value_stack[value_stack_size++];
    vs.path = path;
    vs.ss = s;
    return vs;
  }

void Context::pop_semantic_values_scope() { value_stack_size--; }

void Context::push_args(std::vector<std::shared_ptr<Ope>> &&args) {
    args_stack.emplace_back(std::move(args));
  }

void Context::pop_args() { args_stack.pop_back(); }

const std::vector<std::shared_ptr<Ope>> &Context::top_args() const {
    return args_stack[args_stack.size() - 1];
  }

Context::Snapshot Context::snapshot(const SemanticValues &vs) const {
    return {vs.size(),        vs.tags.size(), vs.tokens.size(),      vs.sv_,
            vs.choice_count_, vs.choice_,     capture_entries.size()};
  }

void Context::rollback(SemanticValues &vs, const Snapshot &snap) {
    vs.resize(snap.sv_size);
    vs.tags.resize(snap.sv_tags_size);
    vs.tokens.resize(snap.sv_tokens_size);
    vs.sv_ = snap.sv_sv;
    vs.choice_count_ = snap.choice_count;
    vs.choice_ = snap.choice;
    capture_entries.resize(snap.capture_size);
  }

std::pair<size_t, size_t> Context::line_info(const char *cur) const {
    std::call_once(source_line_index_init_, [this]() {
      for (size_t pos = 0; pos < l; pos++) {
        if (s[pos] == '\n') { source_line_index.push_back(pos); }
      }
      source_line_index.push_back(l);
    });

    auto pos = static_cast<size_t>(std::distance(s, cur));

    auto it = std::lower_bound(
        source_line_index.begin(), source_line_index.end(), pos,
        [](size_t element, size_t value) { return element < value; });

    auto id = static_cast<size_t>(std::distance(source_line_index.begin(), it));
    auto off = pos - (id == 0 ? 0 : source_line_index[id - 1] + 1);
    return std::pair(id + 1, off + 1);
  }

Sequence::Sequence(const std::vector<std::shared_ptr<Ope>> &opes) : opes_(opes) {}

Sequence::Sequence(std::vector<std::shared_ptr<Ope>> &&opes) : opes_(std::move(opes)) {}

Sequence::~Sequence() = default;

size_t Sequence::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    // Keyword-guarded identifier fast path:
    // Fuses !ReservedKeyword <identifier> into scan-then-lookup
    if (kw_guard_) {
      if (auto result = parse_keyword_guarded(s, n, vs, c, dt)) {
        return *result;
      }
      // nullopt means prefix keyword match — fall through to normal path
    }
    size_t i = 0;
    for (const auto &ope : opes_) {
      auto len = ope->parse(s + i, n - i, vs, c, dt);
      if (fail(len)) { return len; }
      i += len;
    }
    return i;
  }

std::optional<size_t> Sequence::parse_keyword_guarded(const char *s, size_t n,
                                              SemanticValues &vs, Context &c,
                                              std::any &dt) const {
    const auto &kw = *kw_guard_;
    if (n < 1 || !kw.identifier_first.test(static_cast<unsigned char>(*s))) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
    // Scan identifier using bitset
    size_t id_len = 1;
    while (id_len < n &&
           kw.identifier_rest.test(static_cast<unsigned char>(s[id_len]))) {
      id_len++;
    }
    // Skip keyword matching if identifier length is out of range
    if (id_len >= kw.min_keyword_len && id_len <= kw.max_keyword_len) {
      char lower_buf[64];
      std::unique_ptr<char[]> lower_heap;
      char *lower = lower_buf;
      if (id_len > sizeof(lower_buf)) {
        lower_heap.reset(new char[id_len]);
        lower = lower_heap.get();
      }
      std::transform(s, s + id_len, lower, [&c](unsigned char ch) {
        return static_cast<char>(c.tolower_table[ch]);
      });
      std::string_view lower_sv(lower, id_len);

      if (KeywordGuardData::matches_any(kw.exact_keywords, lower_sv)) {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      }
      if (KeywordGuardData::matches_any(kw.prefix_keywords, lower_sv)) {
        return std::nullopt;
      }
    }
    // Success: emit token and consume trailing whitespace
    vs.tokens.emplace_back(std::string_view(s, id_len));
    auto wl = c.skip_whitespace(s + id_len, n - id_len, vs, dt);
    if (fail(wl)) { return wl; }
    return id_len + wl;
  }

PrioritizedChoice::PrioritizedChoice(const std::vector<std::shared_ptr<Ope>> &opes)
      : opes_(opes) {
    is_choice_like = true;
  }

PrioritizedChoice::PrioritizedChoice(std::vector<std::shared_ptr<Ope>> &&opes)
      : opes_(std::move(opes)) {
    is_choice_like = true;
  }

size_t PrioritizedChoice::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    size_t len = static_cast<size_t>(-1);

    if (!for_label_) { c.cut_stack.push_back(false); }
    auto se = scope_exit([&]() {
      if (!for_label_) { c.cut_stack.pop_back(); }
    });

    size_t id = 0;
    for (const auto &ope : opes_) {
      // First-Set filtering: skip if next byte cannot start this alternative
      if (n > 0 && id < first_sets_.size()) {
        const auto &fs = first_sets_[id];
        if (!fs.any_char && !fs.can_be_empty &&
            !fs.chars.test(static_cast<unsigned char>(*s))) {
          if ((c.log || c.error_reporter) &&
              (fs.first_literal || fs.first_rule)) {
            if (c.error_info.error_pos <= s) {
              if (c.error_info.error_pos < s || !(id > 0)) {
                c.error_info.error_pos = s;
                c.error_info.expected_tokens.clear();
              }
              if (fs.first_literal) {
                c.error_info.add(fs.first_literal, nullptr);
              } else {
                c.error_info.add(nullptr, fs.first_rule);
              }
            }
          }
          id++;
          continue;
        }
      }

      if (!c.cut_stack.empty()) { c.cut_stack.back() = false; }

      auto snap = c.snapshot(vs);
      c.error_info.keep_previous_token = id > 0;

      len = ope->parse(s, n, vs, c, dt);

      if (success(len)) {
        vs.choice_count_ = opes_.size();
        vs.choice_ = id;
        break;
      }

      c.rollback(vs, snap);

      if (!c.cut_stack.empty() && c.cut_stack.back()) { break; }

      id++;
    }

    c.error_info.keep_previous_token = false;
    return len;
  }

size_t PrioritizedChoice::size() const { return opes_.size(); }

Repetition::Repetition(const std::shared_ptr<Ope> &ope, size_t min, size_t max)
      : ope_(ope), min_(min), max_(max) {}

size_t Repetition::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    // ISpan fast path: tight loop for ASCII CharacterClass repetition.
    // Safe because each ASCII match is exactly 1 byte, so byte count == match
    // count.
    if (span_bitset_) {
      const auto &bitset = *span_bitset_;
      size_t i = 0;
      if (max_ == std::numeric_limits<size_t>::max()) {
        // Unbounded repetition (*, +): no per-iteration max check
        while (i < n && bitset.test(static_cast<unsigned char>(s[i]))) {
          i++;
        }
      } else {
        auto limit = std::min(n, max_);
        while (i < limit && bitset.test(static_cast<unsigned char>(s[i]))) {
          i++;
        }
      }
      if (i < min_) {
        c.set_error_pos(s + i);
        return static_cast<size_t>(-1);
      }
      return i;
    }

    size_t count = 0;
    size_t i = 0;
    while (count < min_) {
      auto len = ope_->parse(s + i, n - i, vs, c, dt);
      if (fail(len)) { return len; }
      i += len;
      count++;
    }

    while (count < max_) {
      auto snap = c.snapshot(vs);
      auto len = ope_->parse(s + i, n - i, vs, c, dt);
      if (fail(len)) {
        c.rollback(vs, snap);
        break;
      }
      i += len;
      count++;
    }
    return i;
  }

bool Repetition::is_zom() const {
    return min_ == 0 && max_ == std::numeric_limits<size_t>::max();
  }

std::shared_ptr<Repetition> Repetition::zom(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Repetition>(ope, 0,
                                        std::numeric_limits<size_t>::max());
  }

std::shared_ptr<Repetition> Repetition::oom(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Repetition>(ope, 1,
                                        std::numeric_limits<size_t>::max());
  }

std::shared_ptr<Repetition> Repetition::opt(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Repetition>(ope, 0, 1);
  }

AndPredicate::AndPredicate(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

size_t AndPredicate::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    auto snap = c.snapshot(vs);
    auto len = ope_->parse(s, n, vs, c, dt);
    c.rollback(vs, snap); // Always rollback — predicates consume nothing
    if (success(len)) {
      return 0;
    } else {
      return len;
    }
  }

NotPredicate::NotPredicate(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

size_t NotPredicate::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    auto snap = c.snapshot(vs);
    auto len = ope_->parse(s, n, vs, c, dt);
    c.rollback(vs, snap); // Always rollback — predicates consume nothing
    if (success(len)) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    } else {
      return 0;
    }
  }

Dictionary::Dictionary(const std::vector<std::string> &v, bool ignore_case)
      : trie_(v, ignore_case) {
    is_choice_like = true;
  }

LiteralString::LiteralString(std::string &&s, bool ignore_case)
      : lit_(std::move(s)), ignore_case_(ignore_case),
        lower_lit_(ignore_case ? to_lower(lit_) : std::string()),
        is_word_(false) {}

LiteralString::LiteralString(const std::string &s, bool ignore_case)
      : lit_(s), ignore_case_(ignore_case),
        lower_lit_(ignore_case ? to_lower(lit_) : std::string()),
        is_word_(false) {}

CharacterClass::CharacterClass(const std::string &s, bool negated, bool ignore_case)
      : negated_(negated), ignore_case_(ignore_case) {
    auto chars = decode(s.data(), s.length());
    auto i = 0u;
    while (i < chars.size()) {
      if (i + 2 < chars.size() && chars[i + 1] == '-') {
        auto cp1 = chars[i];
        auto cp2 = chars[i + 2];
        ranges_.emplace_back(std::pair(cp1, cp2));
        i += 3;
      } else {
        auto cp = chars[i];
        ranges_.emplace_back(std::pair(cp, cp));
        i += 1;
      }
    }
    assert(!ranges_.empty());
    setup_ascii_bitset();
  }

CharacterClass::CharacterClass(const std::vector<std::pair<char32_t, char32_t>> &ranges,
                 bool negated, bool ignore_case)
      : ranges_(ranges), negated_(negated), ignore_case_(ignore_case) {
    assert(!ranges_.empty());
    setup_ascii_bitset();
  }

size_t CharacterClass::parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const {
    if (n < 1) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }

    char32_t cp = 0;
    auto len = decode_codepoint(s, n, cp);

    for (const auto &range : ranges_) {
      if (in_range(range, cp)) {
        if (negated_) {
          c.set_error_pos(s);
          return static_cast<size_t>(-1);
        } else {
          return len;
        }
      }
    }

    if (negated_) {
      return len;
    } else {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
  }

bool CharacterClass::is_ascii_only() const { return is_ascii_only_; }

const std::bitset<256> &CharacterClass::ascii_bitset() const { return ascii_bitset_; }

bool CharacterClass::in_range(const std::pair<char32_t, char32_t> &range, char32_t cp) const {
    if (ignore_case_) {
      auto cpl = std::tolower(cp);
      return std::tolower(range.first) <= cpl &&
             cpl <= std::tolower(range.second);
    } else {
      return range.first <= cp && cp <= range.second;
    }
  }

void CharacterClass::setup_ascii_bitset() {
    if (negated_) { return; } // negated classes can match non-ASCII
    for (const auto &[lo, hi] : ranges_) {
      if (lo > 0x7F || hi > 0x7F) { return; }
    }
    is_ascii_only_ = true;
    for (const auto &[lo, hi] : ranges_) {
      for (auto cp = lo; cp <= hi; cp++) {
        auto ch = static_cast<unsigned char>(cp);
        ascii_bitset_.set(ch);
        if (ignore_case_) {
          ascii_bitset_.set(static_cast<unsigned char>(std::toupper(ch)));
          ascii_bitset_.set(static_cast<unsigned char>(std::tolower(ch)));
        }
      }
    }
  }

Character::Character(char32_t ch) : ch_(ch) {}

size_t Character::parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const {
    if (n < 1) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }

    char32_t cp = 0;
    auto len = decode_codepoint(s, n, cp);

    if (cp != ch_) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
    return len;
  }

size_t AnyCharacter::parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const {
    auto len = codepoint_length(s, n);
    if (len < 1) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
    return len;
  }

CaptureScope::CaptureScope(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

size_t CaptureScope::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    auto cap_snap = c.capture_entries.size();
    auto len = ope_->parse(s, n, vs, c, dt);
    c.capture_entries.resize(cap_snap); // Always rollback (isolation)
    return len;
  }

Capture::Capture(const std::shared_ptr<Ope> &ope, MatchAction ma)
      : ope_(ope), match_action_(ma) {}

size_t Capture::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    auto len = ope_->parse(s, n, vs, c, dt);
    if (success(len) && match_action_) { match_action_(s, len, c); }
    return len;
  }

TokenBoundary::TokenBoundary(const std::shared_ptr<Ope> &ope) : ope_(ope) {
    is_token_boundary = true;
  }

Ignore::Ignore(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

size_t Ignore::parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any &dt) const {
    auto &chvs = c.push_semantic_values_scope();
    auto se = scope_exit([&]() { c.pop_semantic_values_scope(); });
    return ope_->parse(s, n, chvs, c, dt);
  }

User::User(Parser fn) : fn_(fn) {}

size_t User::parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context & /*c*/, std::any &dt) const {
    assert(fn_);
    return fn_(s, n, vs, dt);
  }

WeakHolder::WeakHolder(const std::shared_ptr<Ope> &ope) : weak_(ope) {}

size_t WeakHolder::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    auto ope = weak_.lock();
    assert(ope);
    return ope->parse(s, n, vs, c, dt);
  }

Holder::Holder(Definition *outer) : outer_(outer) {}

Whitespace::Whitespace(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

size_t Whitespace::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    if (c.in_whitespace) { return 0; }
    c.in_whitespace = true;
    auto se = scope_exit([&]() { c.in_whitespace = false; });
    return ope_->parse(s, n, vs, c, dt);
  }

BackReference::BackReference(std::string &&name) : name_(std::move(name)) {}

BackReference::BackReference(const std::string &name) : name_(name) {}

PrecedenceClimbing::PrecedenceClimbing(const std::shared_ptr<Ope> &atom,
                     const std::shared_ptr<Ope> &binop, const BinOpeInfo &info,
                     const Definition &rule)
      : atom_(atom), binop_(binop), info_(info), rule_(rule) {}

size_t PrecedenceClimbing::parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const {
    return parse_expression(s, n, vs, c, dt, 0);
  }

Recovery::Recovery(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

size_t Cut::parse_core(const char * /*s*/, size_t /*n*/, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const {
    if (!c.cut_stack.empty()) { c.cut_stack.back() = true; }
    return 0;
  }

Definition::Definition() : holder_(std::make_shared<Holder>(this)) {}

Definition::Definition(const Definition &rhs) : name(rhs.name), holder_(rhs.holder_) {
    holder_->outer_ = this;
  }

Definition::Definition(const std::shared_ptr<Ope> &ope)
      : holder_(std::make_shared<Holder>(this)) {
    *this <= ope;
  }

Definition::operator std::shared_ptr<Ope>() {
    return std::make_shared<WeakHolder>(holder_);
  }

Definition &Definition::operator<=(const std::shared_ptr<Ope> &ope) {
    holder_->ope_ = ope;
    return *this;
  }

Definition::Result Definition::parse(const char *s, size_t n, const char *path,
               Log log,
               ErrorReporter error_reporter) const {
    SemanticValues vs;
    std::any dt;
    return parse_core(s, n, vs, dt, path, log, error_reporter);
  }

Definition::Result Definition::parse(const char *s, const char *path, Log log,
               ErrorReporter error_reporter) const {
    auto n = strlen(s);
    return parse(s, n, path, log, error_reporter);
  }

Definition::Result Definition::parse(const char *s, size_t n, std::any &dt,
               const char *path, Log log,
               ErrorReporter error_reporter) const {
    SemanticValues vs;
    return parse_core(s, n, vs, dt, path, log, error_reporter);
  }

Definition::Result Definition::parse(const char *s, std::any &dt, const char *path,
               Log log,
               ErrorReporter error_reporter) const {
    auto n = strlen(s);
    return parse(s, n, dt, path, log, error_reporter);
  }

void Definition::operator=(Action a) { action = a; }

Definition &Definition::operator~() {
    ignoreSemanticValue = true;
    return *this;
  }

void Definition::accept(Ope::Visitor &v) { holder_->accept(v); }

std::shared_ptr<Ope> Definition::get_core_operator() const { return holder_->ope_; }

bool Definition::is_token() const {
    std::call_once(is_token_init_, [this]() {
      is_token_ = TokenChecker::is_token(*get_core_operator());
    });
    return is_token_;
  }

void Definition::initialize_definition_ids() const {
    std::call_once(definition_ids_init_, [&]() {
      AssignIDToDefinition vis;
      holder_->accept(vis);
      if (whitespaceOpe) { whitespaceOpe->accept(vis); }
      if (wordOpe) { wordOpe->accept(vis); }
      definition_ids_.swap(vis.ids);
    });
  }

Definition::Result Definition::parse_core(const char *s, size_t n, SemanticValues &vs, std::any &dt,
                    const char *path, Log log,
                    ErrorReporter error_reporter) const {
    initialize_definition_ids();

    std::shared_ptr<Ope> ope = holder_;

    std::any trace_data;
    if (tracer_start) { tracer_start(trace_data); }
    auto se = scope_exit([&]() {
      if (tracer_end) { tracer_end(trace_data); }
    });

    const std::vector<int32_t> *packrat_index;
    size_t packrat_cached_count = 0;
    if (enablePackratParsing) {
      initialize_packrat_filter();
      if (!packrat_index_.empty()) {
        packrat_index = &packrat_index_;
        packrat_cached_count = packrat_cached_count_;
      } else {
        packrat_cached_count = definition_ids_.size();
      }
    }

    Context c(path, s, n, definition_ids_.size(), whitespaceOpe, wordOpe,
              enablePackratParsing, tracer_enter, tracer_leave, trace_data,
              verbose_trace, log, error_reporter, packrat_index,
              packrat_cached_count);

    if (collect_packrat_stats) {
      packrat_stats_.resize(definition_ids_.size());
      c.packrat_stats = &packrat_stats_;
    }

    size_t i = 0;

    if (whitespaceOpe) {
      auto save_ignore_trace_state = c.ignore_trace_state;
      c.ignore_trace_state = !c.verbose_trace;
      auto se =
          scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

      auto len = whitespaceOpe->parse(s, n, vs, c, dt);
      if (fail(len)) { return Result{false, c.recovered, i, c.error_info}; }

      i = len;
    }

    auto len = ope->parse(s + i, n - i, vs, c, dt);
    auto ret = success(len);
    if (ret) {
      i += len;
      if (eoi_check) {
        if (i < n) {
          if (c.error_info.error_pos - c.s < s + i - c.s) {
            c.error_info.message_pos = s + i;
            c.error_info.message = "expected end of input";
          }
          ret = false;
        }
      }
    }
    return Result{ret, c.recovered, i, c.error_info};
  }

/*
 * Helpers
 */

size_t codepoint_length(const char *s8, size_t l) {
  if (l) {
    auto b = static_cast<uint8_t>(s8[0]);
    if ((b & 0x80) == 0) {
      return 1;
    } else if ((b & 0xE0) == 0xC0 && l >= 2) {
      return 2;
    } else if ((b & 0xF0) == 0xE0 && l >= 3) {
      return 3;
    } else if ((b & 0xF8) == 0xF0 && l >= 4) {
      return 4;
    }
  }
  return 0;
}

size_t codepoint_count(const char *s8, size_t l) {
  size_t count = 0;
  for (size_t i = 0; i < l;) {
    auto len = codepoint_length(s8 + i, l - i);
    if (len == 0) {
      // Invalid UTF-8 byte, treat as single byte to avoid infinite loop
      len = 1;
    }
    i += len;
    count++;
  }
  return count;
}

size_t encode_codepoint(char32_t cp, char *buff) {
  if (cp < 0x0080) {
    buff[0] = static_cast<char>(cp & 0x7F);
    return 1;
  } else if (cp < 0x0800) {
    buff[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
    buff[1] = static_cast<char>(0x80 | (cp & 0x3F));
    return 2;
  } else if (cp < 0xD800) {
    buff[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (cp & 0x3F));
    return 3;
  } else if (cp < 0xE000) {
    // D800 - DFFF is invalid...
    return 0;
  } else if (cp < 0x10000) {
    buff[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (cp & 0x3F));
    return 3;
  } else if (cp < 0x110000) {
    buff[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x7));
    buff[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    buff[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    buff[3] = static_cast<char>(0x80 | (cp & 0x3F));
    return 4;
  }
  return 0;
}

std::string encode_codepoint(char32_t cp) {
  char buff[4];
  auto l = encode_codepoint(cp, buff);
  return std::string(buff, l);
}

bool decode_codepoint(const char *s8, size_t l, size_t &bytes,
                             char32_t &cp) {
  if (l) {
    auto b = static_cast<uint8_t>(s8[0]);
    if ((b & 0x80) == 0) {
      bytes = 1;
      cp = b;
      return true;
    } else if ((b & 0xE0) == 0xC0) {
      if (l >= 2) {
        bytes = 2;
        cp = ((static_cast<char32_t>(s8[0] & 0x1F)) << 6) |
             (static_cast<char32_t>(s8[1] & 0x3F));
        return true;
      }
    } else if ((b & 0xF0) == 0xE0) {
      if (l >= 3) {
        bytes = 3;
        cp = ((static_cast<char32_t>(s8[0] & 0x0F)) << 12) |
             ((static_cast<char32_t>(s8[1] & 0x3F)) << 6) |
             (static_cast<char32_t>(s8[2] & 0x3F));
        return true;
      }
    } else if ((b & 0xF8) == 0xF0) {
      if (l >= 4) {
        bytes = 4;
        cp = ((static_cast<char32_t>(s8[0] & 0x07)) << 18) |
             ((static_cast<char32_t>(s8[1] & 0x3F)) << 12) |
             ((static_cast<char32_t>(s8[2] & 0x3F)) << 6) |
             (static_cast<char32_t>(s8[3] & 0x3F));
        return true;
      }
    }
  }
  return false;
}

size_t decode_codepoint(const char *s8, size_t l, char32_t &cp) {
  size_t bytes;
  if (decode_codepoint(s8, l, bytes, cp)) { return bytes; }
  return 0;
}

char32_t decode_codepoint(const char *s8, size_t l) {
  char32_t cp = 0;
  decode_codepoint(s8, l, cp);
  return cp;
}

std::u32string decode(const char *s8, size_t l) {
  std::u32string out;
  size_t i = 0;
  while (i < l) {
    auto beg = i++;
    while (i < l && (s8[i] & 0xc0) == 0x80) {
      i++;
    }
    out += decode_codepoint(&s8[beg], (i - beg));
  }
  return out;
}

std::string escape_characters(const char *s, size_t n) {
  std::string str;
  for (size_t i = 0; i < n; i++) {
    auto c = s[i];
    switch (c) {
    case '\f': str += "\\f"; break;
    case '\n': str += "\\n"; break;
    case '\r': str += "\\r"; break;
    case '\t': str += "\\t"; break;
    case '\v': str += "\\v"; break;
    default: str += c; break;
    }
  }
  return str;
}

std::string escape_characters(std::string_view sv) {
  return escape_characters(sv.data(), sv.size());
}

bool is_hex(char c, int &v) {
  if ('0' <= c && c <= '9') {
    v = c - '0';
    return true;
  } else if ('a' <= c && c <= 'f') {
    v = c - 'a' + 10;
    return true;
  } else if ('A' <= c && c <= 'F') {
    v = c - 'A' + 10;
    return true;
  }
  return false;
}

bool is_digit(char c, int &v) {
  if ('0' <= c && c <= '9') {
    v = c - '0';
    return true;
  }
  return false;
}

std::pair<int, size_t> parse_hex_number(const char *s, size_t n,
                                               size_t i) {
  int ret = 0;
  int val;
  while (i < n && is_hex(s[i], val)) {
    ret = static_cast<int>(ret * 16 + val);
    i++;
  }
  return std::pair(ret, i);
}

std::pair<int, size_t> parse_octal_number(const char *s, size_t n,
                                                 size_t i) {
  int ret = 0;
  int val;
  while (i < n && is_digit(s[i], val)) {
    ret = static_cast<int>(ret * 8 + val);
    i++;
  }
  return std::pair(ret, i);
}

std::string resolve_escape_sequence(const char *s, size_t n) {
  std::string r;
  r.reserve(n);

  size_t i = 0;
  while (i < n) {
    auto ch = s[i];
    if (ch == '\\') {
      i++;
      assert(i < n);

      switch (s[i]) {
      case 'f':
        r += '\f';
        i++;
        break;
      case 'n':
        r += '\n';
        i++;
        break;
      case 'r':
        r += '\r';
        i++;
        break;
      case 't':
        r += '\t';
        i++;
        break;
      case 'v':
        r += '\v';
        i++;
        break;
      case '\'':
        r += '\'';
        i++;
        break;
      case '"':
        r += '"';
        i++;
        break;
      case '[':
        r += '[';
        i++;
        break;
      case ']':
        r += ']';
        i++;
        break;
      case '^':
        r += '^';
        i++;
        break;
      case '-':
        r += '-';
        i++;
        break;
      case '\\':
        r += '\\';
        i++;
        break;
      case 'x':
      case 'u': {
        char32_t cp;
        std::tie(cp, i) = parse_hex_number(s, n, i + 1);
        r += encode_codepoint(cp);
        break;
      }
      default: {
        char32_t cp;
        std::tie(cp, i) = parse_octal_number(s, n, i);
        r += encode_codepoint(cp);
        break;
      }
      }
    } else {
      r += ch;
      i++;
    }
  }
  return r;
}

const std::vector<std::pair<char32_t, char32_t>> *
predefined_character_class(std::string_view name) {
  static const std::map<std::string_view,
                        std::vector<std::pair<char32_t, char32_t>>>
      table = {
          {"alnum", {{'0', '9'}, {'A', 'Z'}, {'a', 'z'}}},
          {"alpha", {{'A', 'Z'}, {'a', 'z'}}},
          {"ascii", {{0x00, 0x7F}}},
          {"blank", {{'\t', '\t'}, {' ', ' '}}},
          {"cntrl", {{0x00, 0x1F}, {0x7F, 0x7F}}},
          {"digit", {{'0', '9'}}},
          {"graph", {{0x21, 0x7E}}},
          {"lower", {{'a', 'z'}}},
          {"print", {{0x20, 0x7E}}},
          {"punct", {{0x21, 0x2F}, {0x3A, 0x40}, {0x5B, 0x60}, {0x7B, 0x7E}}},
          {"space", {{'\t', '\r'}, {' ', ' '}}},
          {"upper", {{'A', 'Z'}}},
          {"word", {{'0', '9'}, {'A', 'Z'}, {'_', '_'}, {'a', 'z'}}},
          {"xdigit", {{'0', '9'}, {'A', 'F'}, {'a', 'f'}}},
      };
  auto it = table.find(name);
  return it != table.end() ? &it->second : nullptr;
}

std::vector<std::pair<char32_t, char32_t>> complement_character_ranges(
    const std::vector<std::pair<char32_t, char32_t>> &ranges) {
  std::vector<std::pair<char32_t, char32_t>> r;
  char32_t next = 0;
  for (const auto &[lo, hi] : ranges) {
    if (lo > next) { r.emplace_back(next, lo - 1); }
    next = hi + 1;
  }
  if (next <= 0x10FFFF) { r.emplace_back(next, 0x10FFFF); }
  return r;
}

std::string to_lower(std::string s) {
  for (auto &c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

std::pair<size_t, size_t> line_info(const char *start, const char *cur) {
  auto p = start;
  auto col_ptr = p;
  auto no = 1;

  while (p < cur) {
    if (*p == '\n') {
      no++;
      col_ptr = p + 1;
    }
    p++;
  }

  auto col = codepoint_count(col_ptr, p - col_ptr) + 1;

  return std::pair(no, col);
}

bool success(size_t len) { return len != static_cast<size_t>(-1); }

bool fail(size_t len) { return len == static_cast<size_t>(-1); }

std::shared_ptr<Ope> zom(const std::shared_ptr<Ope> &ope) {
  return Repetition::zom(ope);
}

std::shared_ptr<Ope> oom(const std::shared_ptr<Ope> &ope) {
  return Repetition::oom(ope);
}

std::shared_ptr<Ope> opt(const std::shared_ptr<Ope> &ope) {
  return Repetition::opt(ope);
}

std::shared_ptr<Ope> rep(const std::shared_ptr<Ope> &ope, size_t min,
                                size_t max) {
  return std::make_shared<Repetition>(ope, min, max);
}

std::shared_ptr<Ope> apd(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<AndPredicate>(ope);
}

std::shared_ptr<Ope> npd(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<NotPredicate>(ope);
}

std::shared_ptr<Ope> dic(const std::vector<std::string> &v,
                                bool ignore_case) {
  return std::make_shared<Dictionary>(v, ignore_case);
}

std::shared_ptr<Ope> lit(std::string &&s) {
  return std::make_shared<LiteralString>(s, false);
}

std::shared_ptr<Ope> liti(std::string &&s) {
  return std::make_shared<LiteralString>(s, true);
}

std::shared_ptr<Ope> cls(const std::string &s) {
  return std::make_shared<CharacterClass>(s, false, false);
}

std::shared_ptr<Ope>
cls(const std::vector<std::pair<char32_t, char32_t>> &ranges,
    bool ignore_case) {
  return std::make_shared<CharacterClass>(ranges, false, ignore_case);
}

std::shared_ptr<Ope> ncls(const std::string &s) {
  return std::make_shared<CharacterClass>(s, true, false);
}

std::shared_ptr<Ope>
ncls(const std::vector<std::pair<char32_t, char32_t>> &ranges,
     bool ignore_case) {
  return std::make_shared<CharacterClass>(ranges, true, ignore_case);
}

std::shared_ptr<Ope> chr(char32_t dt) {
  return std::make_shared<Character>(dt);
}

std::shared_ptr<Ope> dot() { return std::make_shared<AnyCharacter>(); }

std::shared_ptr<Ope> csc(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<CaptureScope>(ope);
}

std::shared_ptr<Ope> cap(const std::shared_ptr<Ope> &ope,
                                Capture::MatchAction ma) {
  return std::make_shared<Capture>(ope, ma);
}

std::shared_ptr<Ope> tok(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<TokenBoundary>(ope);
}

std::shared_ptr<Ope> ign(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<Ignore>(ope);
}

std::shared_ptr<Ope>
usr(std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                         std::any &dt)>
        fn) {
  return std::make_shared<User>(fn);
}

std::shared_ptr<Ope> ref(const Grammar &grammar, const std::string &name,
                                const char *s, bool is_macro,
                                const std::vector<std::shared_ptr<Ope>> &args) {
  return std::make_shared<Reference>(grammar, name, s, is_macro, args);
}

std::shared_ptr<Ope> wsp(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<Whitespace>(std::make_shared<Ignore>(ope));
}

std::shared_ptr<Ope> bkr(std::string &&name) {
  return std::make_shared<BackReference>(name);
}

std::shared_ptr<Ope> pre(const std::shared_ptr<Ope> &atom,
                                const std::shared_ptr<Ope> &binop,
                                const PrecedenceClimbing::BinOpeInfo &info,
                                const Definition &rule) {
  return std::make_shared<PrecedenceClimbing>(atom, binop, info, rule);
}

std::shared_ptr<Ope> rec(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<Recovery>(ope);
}

std::shared_ptr<Ope> cut() { return std::make_shared<Cut>(); }

/*
 * Implementations
 */

size_t parse_literal(const char *s, size_t n, SemanticValues &vs,
                            Context &c, std::any &dt, const std::string &lit,
                            std::once_flag &init_is_word, bool &is_word,
                            bool ignore_case, const std::string &lower_lit) {
  size_t i = 0;
  for (; i < lit.size(); i++) {
    if (i >= n ||
        (ignore_case
             ? (static_cast<char>(
                    c.tolower_table[static_cast<unsigned char>(s[i])]) !=
                lower_lit[i])
             : (s[i] != lit[i]))) {
      c.set_error_pos(s, lit.data());
      return static_cast<size_t>(-1);
    }
  }

  // Word check
  if (c.wordOpe) {
    auto save_ignore_trace_state = c.ignore_trace_state;
    c.ignore_trace_state = !c.verbose_trace;
    auto se =
        scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

    std::call_once(init_is_word, [&]() {
      SemanticValues dummy_vs;
      Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr,
                      nullptr, nullptr, false, nullptr);
      std::any dummy_dt;

      auto len =
          c.wordOpe->parse(lit.data(), lit.size(), dummy_vs, dummy_c, dummy_dt);
      is_word = success(len);
    });

    if (is_word) {
      SemanticValues dummy_vs;
      Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr,
                      nullptr, nullptr, false, nullptr);
      std::any dummy_dt;

      NotPredicate ope(c.wordOpe);
      auto len = ope.parse(s + i, n - i, dummy_vs, dummy_c, dummy_dt);
      if (fail(len)) {
        c.set_error_pos(s, lit.data());
        return len;
      }
      i += len;
    }
  }

  // Skip whitespace
  auto wl = c.skip_whitespace(s + i, n - i, vs, dt);
  if (fail(wl)) { return wl; }
  i += wl;

  return i;
}

std::pair<size_t, size_t> SemanticValues::line_info() const {
  assert(c_);
  return c_->line_info(sv_.data());
}

void ErrorInfo::output_log(const Log &log, const ErrorReporter &reporter,
                                  const char *s, size_t n) {
  if (message_pos) {
    if (message_pos > last_output_pos) {
      last_output_pos = message_pos;
      auto line = line_info(s, message_pos);
      std::string msg;
      auto unexpected_token = heuristic_error_token(s, n, message_pos);
      if (!unexpected_token.empty()) {
        msg = replace_all(message, "%t", unexpected_token);

        auto unexpected_char = unexpected_token.substr(
            0,
            codepoint_length(unexpected_token.data(), unexpected_token.size()));

        msg = replace_all(msg, "%c", unexpected_char);
      } else {
        msg = message;
      }
      if (reporter) {
        ErrorReport report;
        report.line = line.first;
        report.col = line.second;
        report.position = static_cast<size_t>(message_pos - s);
        report.unexpected_token = unexpected_token;
        report.message = msg;
        report.label = label;
        reporter(report);
      }
      if (log) { log(line.first, line.second, msg, label); }
    }
  } else if (error_pos) {
    if (error_pos > last_output_pos) {
      last_output_pos = error_pos;
      auto line = line_info(s, error_pos);

      ErrorReport report;
      report.line = line.first;
      report.col = line.second;
      report.position = static_cast<size_t>(error_pos - s);

      std::string msg;
      if (expected_tokens.empty()) {
        msg = "syntax error.";
      } else {
        msg = "syntax error";

        // unexpected token
        if (auto unexpected_token = heuristic_error_token(s, n, error_pos);
            !unexpected_token.empty()) {
          msg += ", unexpected '";
          msg += unexpected_token;
          msg += "'";
          report.unexpected_token = unexpected_token;
        }

        auto first_item = true;
        size_t i = 0;
        while (i < expected_tokens.size()) {
          auto [error_literal, error_rule] = expected_tokens[i];

          // Skip rules start with '_'
          if (!(error_rule && error_rule->name[0] == '_')) {
            msg += (first_item ? ", expecting " : ", ");
            if (error_literal) {
              msg += "'";
              msg += error_literal;
              msg += "'";
              report.expected_literals.emplace_back(error_literal);
            } else {
              msg += "<" + error_rule->name + ">";
              if (label.empty()) { label = error_rule->name; }
              report.expected_rules.emplace_back(error_rule->name);
            }
            first_item = false;
          }

          i++;
        }
        msg += ".";
      }
      if (reporter) {
        report.label = label;
        reporter(report);
      }
      if (log) { log(line.first, line.second, msg, label); }
    }
  }
}

size_t Context::skip_whitespace(const char *a_s, size_t n,
                                       SemanticValues &vs, std::any &dt) {
  if (in_token_boundary_count || !whitespaceOpe) { return 0; }
  auto save = ignore_trace_state;
  ignore_trace_state = !verbose_trace;
  auto se = scope_exit([&]() { ignore_trace_state = save; });
  return whitespaceOpe->parse(a_s, n, vs, *this, dt);
}

void Context::set_error_pos(const char *a_s, const char *literal) {
  if (log || error_reporter) {
    if (error_info.error_pos <= a_s) {
      if (error_info.error_pos < a_s || !error_info.keep_previous_token) {
        error_info.error_pos = a_s;
        error_info.expected_tokens.clear();
      }

      const char *error_literal = nullptr;
      const Definition *error_rule = nullptr;

      if (literal) {
        error_literal = literal;
      } else if (!rule_stack.empty()) {
        auto rule = rule_stack.back();
        auto ope = rule->get_core_operator();
        if (auto token = FindLiteralToken::token(*ope);
            token && token[0] != '\0') {
          error_literal = token;
        }
      }

      for (auto r : rule_stack) {
        error_rule = r;
        if (r->is_token()) { break; }
      }

      if (error_literal || error_rule) {
        error_info.add(error_literal, error_rule);
      }
    }
  }
}

void Context::trace_enter(const Ope &ope, const char *a_s, size_t n,
                                 const SemanticValues &vs, std::any &dt) {
  trace_ids.push_back(next_trace_id++);
  tracer_enter(ope, a_s, n, vs, *this, dt, trace_data);
}

void Context::trace_leave(const Ope &ope, const char *a_s, size_t n,
                                 const SemanticValues &vs, std::any &dt,
                                 size_t len) {
  tracer_leave(ope, a_s, n, vs, *this, dt, len, trace_data);
  trace_ids.pop_back();
}

bool Context::is_traceable(const Ope &ope) const {
  if (has_tracer) {
    if (ignore_trace_state) { return false; }
    return !dynamic_cast<const peg::Reference *>(&ope);
  }
  return false;
}

size_t Ope::parse(const char *s, size_t n, SemanticValues &vs,
                         Context &c, std::any &dt) const {
  if (c.is_traceable(*this)) {
    c.trace_enter(*this, s, n, vs, dt);
    auto len = parse_core(s, n, vs, c, dt);
    c.trace_leave(*this, s, n, vs, dt, len);
    return len;
  }
  return parse_core(s, n, vs, c, dt);
}

size_t Dictionary::parse_core(const char *s, size_t n,
                                     SemanticValues &vs, Context &c,
                                     std::any &dt) const {
  size_t id;
  auto i = trie_.match(s, n, id);

  if (i == 0) {
    c.set_error_pos(s);
    return static_cast<size_t>(-1);
  }

  vs.choice_count_ = trie_.items_count();
  vs.choice_ = id;

  // Word check
  if (c.wordOpe) {
    auto save_ignore_trace_state = c.ignore_trace_state;
    c.ignore_trace_state = !c.verbose_trace;
    auto se =
        scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

    {
      SemanticValues dummy_vs;
      Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr,
                      nullptr, nullptr, false, nullptr);
      std::any dummy_dt;

      NotPredicate ope(c.wordOpe);
      auto len = ope.parse(s + i, n - i, dummy_vs, dummy_c, dummy_dt);
      if (fail(len)) {
        c.set_error_pos(s);
        return len;
      }
      i += len;
    }
  }

  // Skip whitespace
  auto wl = c.skip_whitespace(s + i, n - i, vs, dt);
  if (fail(wl)) { return wl; }
  i += wl;

  return i;
}

size_t LiteralString::parse_core(const char *s, size_t n,
                                        SemanticValues &vs, Context &c,
                                        std::any &dt) const {
  return parse_literal(s, n, vs, c, dt, lit_, init_is_word_, is_word_,
                       ignore_case_, lower_lit_);
}

size_t TokenBoundary::parse_core(const char *s, size_t n,
                                        SemanticValues &vs, Context &c,
                                        std::any &dt) const {
  auto save_ignore_trace_state = c.ignore_trace_state;
  c.ignore_trace_state = !c.verbose_trace;
  auto se =
      scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

  size_t len;
  {
    c.in_token_boundary_count++;
    auto se = scope_exit([&]() { c.in_token_boundary_count--; });
    len = ope_->parse(s, n, vs, c, dt);
  }

  if (success(len)) {
    vs.tokens.emplace_back(std::string_view(s, len));

    auto wl = c.skip_whitespace(s + len, n - len, vs, dt);
    if (fail(wl)) { return wl; }
    len += wl;
  }
  return len;
}

// Resolve `%{name}` placeholders in a custom error message against the
// named captures recorded so far ($name<...>). Unknown names resolve to an
// empty string. `%t` / `%c` are resolved later, at log-output time.
std::string resolve_capture_placeholders(const std::string &msg,
                                                const Context &c) {
  auto pos = msg.find("%{");
  if (pos == std::string::npos) { return msg; }

  std::string r;
  size_t i = 0;
  while (pos != std::string::npos) {
    auto end = msg.find('}', pos + 2);
    if (end == std::string::npos) { break; }
    r.append(msg, i, pos - i);
    auto name = std::string_view(msg).substr(pos + 2, end - (pos + 2));
    for (auto it = c.capture_entries.rbegin(); it != c.capture_entries.rend();
         ++it) {
      if (it->first == name) {
        // The captured span can include whitespace skipped after a token
        // boundary; trim it for display.
        auto v = std::string_view(it->second);
        while (!v.empty() &&
               std::isspace(static_cast<unsigned char>(v.back()))) {
          v.remove_suffix(1);
        }
        while (!v.empty() &&
               std::isspace(static_cast<unsigned char>(v.front()))) {
          v.remove_prefix(1);
        }
        r += v;
        break;
      }
    }
    i = end + 1;
    pos = msg.find("%{", i);
  }
  r.append(msg, i, msg.size() - i);
  return r;
}

size_t Holder::parse_core(const char *s, size_t n, SemanticValues &vs,
                                 Context &c, std::any &dt) const {
  if (!ope_) {
    throw std::logic_error("Uninitialized definition ope was used...");
  }

  // Macro reference
  if (outer_->is_macro) {
    c.rule_stack.push_back(outer_);
    auto len = ope_->parse(s, n, vs, c, dt);
    c.rule_stack.pop_back();
    return len;
  }

  size_t len;
  std::any val;

  // Shared parse body: invokes enter/leave callbacks, parses the rule's
  // operator, handles actions/predicates/errors, and calls reduce.
  // Returns {parse_len, parse_val}.
  auto do_parse = [&]() {
    size_t parse_len;
    std::any parse_val;

    if (outer_->enter) { outer_->enter(c, s, n, dt); }
    auto &chvs = c.push_semantic_values_scope();
    auto se = scope_exit([&]() {
      c.pop_semantic_values_scope();
      if (outer_->leave) { outer_->leave(c, s, n, parse_len, parse_val, dt); }
    });

    c.rule_stack.push_back(outer_);
    if (outer_->no_whitespace) {
      {
        c.in_token_boundary_count++;
        auto se2 = scope_exit([&]() { c.in_token_boundary_count--; });
        parse_len = ope_->parse(s, n, chvs, c, dt);
      }
      if (success(parse_len)) {
        auto wl = c.skip_whitespace(s + parse_len, n - parse_len, chvs, dt);
        if (fail(wl)) {
          parse_len = wl;
        } else {
          parse_len += wl;
        }
      }
    } else {
      parse_len = ope_->parse(s, n, chvs, c, dt);
    }
    c.rule_stack.pop_back();

    if (success(parse_len)) {
      chvs.sv_ = std::string_view(s, parse_len);
      chvs.name_ = outer_->name;

      auto ope_ptr = ope_.get();
      if (ope_ptr->is_token_boundary) {
        ope_ptr = static_cast<const peg::TokenBoundary *>(ope_ptr)->ope_.get();
      }
      if (!ope_ptr->is_choice_like) {
        chvs.choice_count_ = 0;
        chvs.choice_ = 0;
      }

      std::string msg;
      std::any predicate_data;
      if (outer_->predicate) {
        if (!outer_->predicate(chvs, dt, msg, predicate_data)) {
          if ((c.log || c.error_reporter) && !msg.empty() &&
              c.error_info.message_pos < s) {
            c.error_info.message_pos = s;
            c.error_info.message = msg;
            c.error_info.label = outer_->name;
          }
          parse_len = static_cast<size_t>(-1);
        }
      }

      if (success(parse_len)) {
        if (!c.recovered) { parse_val = reduce(chvs, dt, predicate_data); }
      } else {
        if ((c.log || c.error_reporter) && !msg.empty() &&
            c.error_info.message_pos < s) {
          c.error_info.message_pos = s;
          c.error_info.message = msg;
          c.error_info.label = outer_->name;
        }
      }
    } else {
      if ((c.log || c.error_reporter) && !outer_->error_message.empty() &&
          c.error_info.message_pos < s) {
        c.error_info.message_pos = s;
        c.error_info.message =
            resolve_capture_placeholders(outer_->error_message, c);
        c.error_info.label = outer_->name;
      }
    }

    return std::make_pair(parse_len, std::move(parse_val));
  };

  if (outer_->is_left_recursive) {
    auto lr_key = std::make_pair(outer_, s);

    // Check LR memo first
    auto it = c.lr_memo.find(lr_key);
    if (it != c.lr_memo.end()) {
      if (success(it->second.len)) {
        len = it->second.len;
        val = it->second.val;
      } else {
        len = static_cast<size_t>(-1);
      }
      // Record that this rule's lr_memo was accessed.
      // Any LR rule currently seeding will know we're in its cycle.
      c.lr_refs_hit.insert(outer_);
    } else {
      // Seed with FAIL
      c.lr_memo[lr_key] = {static_cast<size_t>(-1), {}};

      // Mark as active seed (protects our lr_memo from inner growers)
      c.lr_active_seeds.insert(lr_key);
      auto seed_guard = scope_exit([&]() { c.lr_active_seeds.erase(lr_key); });

      // Track which LR rules are referenced during our parse
      // to identify cycle members
      auto saved_refs = std::move(c.lr_refs_hit);
      c.lr_refs_hit.clear();

      // Initial parse (self-references will hit the FAIL seed)
      auto [initial_len, initial_val] = do_parse();

      // Rules whose lr_memo was hit during our parse are in our cycle.
      // If we detected cycle members, we ourselves are also part of
      // the cycle, so add self — this lets parent seeders see us as
      // a transitive cycle member.
      auto cycle_rules = c.lr_refs_hit;
      if (!cycle_rules.empty()) { cycle_rules.insert(outer_); }

      // Restore parent's refs and propagate cycle info upward
      c.lr_refs_hit = std::move(saved_refs);
      c.lr_refs_hit.insert(cycle_rules.begin(), cycle_rules.end());

      if (!success(initial_len)) {
        // Keep FAIL in lr_memo so we don't re-seed
        len = static_cast<size_t>(-1);
      } else {
        // Got initial seed, now grow
        len = initial_len;
        val = std::move(initial_val);
        c.lr_memo[lr_key] = {len, val};

        while (true) {
          // Clear this rule's packrat cache
          c.clear_packrat_cache(s, outer_->id);

          // Clear lr_memo for cycle-dependent rules at this position,
          // but NOT for rules currently in their own seeding phase
          // (lr_active_seeds) — those are outer growers we must not
          // interfere with.
          for (auto memo_it = c.lr_memo.begin(); memo_it != c.lr_memo.end();) {
            if (memo_it->first.second == s && memo_it->first.first != outer_ &&
                cycle_rules.count(memo_it->first.first) &&
                !c.lr_active_seeds.count(memo_it->first)) {
              memo_it = c.lr_memo.erase(memo_it);
            } else {
              ++memo_it;
            }
          }

          auto [new_len, new_val] = do_parse();

          if (!success(new_len) || new_len <= len) {
            break; // No improvement, done growing
          }

          len = new_len;
          val = std::move(new_val);
          c.lr_memo[lr_key] = {len, val};
        }
      }

      // Write final result to packrat cache (lr_memo entry is kept as
      // the primary lookup for LR rules at this position)
      if (success(len)) { c.write_packrat_cache(s, outer_->id, len, val); }
    }
  } else {
    if (c.enablePackratParsing) {
      // Packrat cache acts as re-entry guard (pre-registered as
      // failure before fn is called).
      c.packrat(s, outer_->id, len, val, [&](std::any &a_val) {
        auto [parse_len, parse_val] = do_parse();
        len = parse_len;
        if (success(len)) { a_val = std::move(parse_val); }
      });
    } else {
      // Without packrat, use lr_memo as re-entry guard to prevent
      // stack overflow from undetected left recursion.
      auto guard_key = std::make_pair(outer_, s);
      if (c.lr_memo.count(guard_key)) {
        len = static_cast<size_t>(-1);
      } else {
        c.lr_memo[guard_key] = {static_cast<size_t>(-1), {}};
        auto [parse_len, parse_val] = do_parse();
        len = parse_len;
        val = std::move(parse_val);
        c.lr_memo.erase(guard_key);
      }
    }
  }

  if (success(len)) {
    if (!outer_->ignoreSemanticValue) {
      vs.emplace_back(std::move(val));
      vs.tags.emplace_back(str2tag(outer_->name));
    }
  }

  return len;
}

std::any Holder::reduce(SemanticValues &vs, std::any &dt,
                               const std::any &predicate_data) const {
  if (outer_->action && !outer_->disable_action) {
    return outer_->action(vs, dt, predicate_data);
  } else if (vs.empty()) {
    return std::any();
  } else {
    return std::move(vs.front());
  }
}

const std::string &Holder::name() const { return outer_->name; }

const std::string &Holder::trace_name() const {
  std::call_once(trace_name_init_,
                 [this]() { trace_name_ = "[" + outer_->name + "]"; });
  return trace_name_;
}

size_t Reference::parse_core(const char *s, size_t n, SemanticValues &vs,
                                    Context &c, std::any &dt) const {
  auto save_ignore_trace_state = c.ignore_trace_state;
  if (rule_ && rule_->ignoreSemanticValue) {
    c.ignore_trace_state = !c.verbose_trace;
  }
  auto se =
      scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

  if (rule_) {
    // Reference rule
    if (rule_->is_macro) {
      // Macro
      FindReference vis(c.top_args(), c.rule_stack.back()->params);

      // Collect arguments
      std::vector<std::shared_ptr<Ope>> args;
      for (const auto &arg : args_) {
        arg->accept(vis);
        args.emplace_back(std::move(vis.found_ope));
      }

      c.push_args(std::move(args));
      auto se = scope_exit([&]() { c.pop_args(); });
      return rule_->holder_->parse(s, n, vs, c, dt);
    } else {
      // Definition
      c.push_args(std::vector<std::shared_ptr<Ope>>());
      auto se2 = scope_exit([&]() { c.pop_args(); });
      return rule_->holder_->parse(s, n, vs, c, dt);
    }
  } else {
    // Reference parameter in macro
    const auto &args = c.top_args();
    return args[iarg_]->parse(s, n, vs, c, dt);
  }
}

std::shared_ptr<Ope> Reference::get_core_operator() const {
  return rule_->holder_;
}

size_t BackReference::parse_core(const char *s, size_t n,
                                        SemanticValues &vs, Context &c,
                                        std::any &dt) const {
  for (auto it = c.capture_entries.rbegin(); it != c.capture_entries.rend();
       ++it) {
    if (it->first == name_) {
      const auto &lit = it->second;
      std::once_flag init_is_word;
      auto is_word = false;
      static const std::string empty;
      return parse_literal(s, n, vs, c, dt, lit, init_is_word, is_word, false,
                           empty);
    }
  }

  c.error_info.message_pos = s;
  c.error_info.message = "undefined back reference '$" + name_ + "'...";
  return static_cast<size_t>(-1);
}

Definition &
PrecedenceClimbing::get_reference_for_binop(Context &c) const {
  if (rule_.is_macro) {
    // Reference parameter in macro
    const auto &args = c.top_args();
    auto iarg = dynamic_cast<Reference &>(*binop_).iarg_;
    auto arg = args[iarg];
    return *dynamic_cast<Reference &>(*arg).rule_;
  }

  return *dynamic_cast<Reference &>(*binop_).rule_;
}

size_t PrecedenceClimbing::parse_expression(const char *s, size_t n,
                                                   SemanticValues &vs,
                                                   Context &c, std::any &dt,
                                                   size_t min_prec) const {
  auto len = atom_->parse(s, n, vs, c, dt);
  if (fail(len)) { return len; }

  std::string tok;
  auto &rule = get_reference_for_binop(c);
  auto action = std::move(rule.action);

  rule.action = [&](SemanticValues &vs2, std::any &dt2,
                    const std::any &predicate_data2) {
    tok = vs2.token();
    if (action) {
      return action(vs2, dt2, predicate_data2);
    } else if (!vs2.empty()) {
      return vs2[0];
    }
    return std::any();
  };
  auto action_se = scope_exit([&]() { rule.action = std::move(action); });

  auto i = len;
  while (i < n) {
    std::vector<std::any> save_values(vs.begin(), vs.end());
    auto save_tokens = vs.tokens;

    auto chvs = c.push_semantic_values_scope();
    auto chlen = binop_->parse(s + i, n - i, chvs, c, dt);
    c.pop_semantic_values_scope();

    if (fail(chlen)) { break; }

    auto it = info_.find(tok);
    if (it == info_.end()) { break; }

    auto level = std::get<0>(it->second);
    auto assoc = std::get<1>(it->second);

    if (level < min_prec) { break; }

    vs.emplace_back(std::move(chvs[0]));
    i += chlen;

    auto next_min_prec = level;
    if (assoc == 'L') { next_min_prec = level + 1; }

    chvs = c.push_semantic_values_scope();
    chlen = parse_expression(s + i, n - i, chvs, c, dt, next_min_prec);
    c.pop_semantic_values_scope();

    if (fail(chlen)) {
      vs.assign(save_values.begin(), save_values.end());
      vs.tokens = save_tokens;
      i = chlen;
      break;
    }

    vs.emplace_back(std::move(chvs[0]));
    i += chlen;

    std::any val;
    if (rule_.action) {
      vs.sv_ = std::string_view(s, i);
      static const std::any empty_predicate_data;
      val = rule_.action(vs, dt, empty_predicate_data);
    } else if (!vs.empty()) {
      val = vs[0];
    }
    vs.clear();
    vs.emplace_back(std::move(val));
  }

  return i;
}

size_t Recovery::parse_core(const char *s, size_t n,
                                   SemanticValues & /*vs*/, Context &c,
                                   std::any & /*dt*/) const {
  const auto &rule = dynamic_cast<Reference &>(*ope_);

  // Custom error message
  if (c.log || c.error_reporter) {
    auto label = dynamic_cast<Reference *>(rule.args_[0].get());
    if (label && !label->rule_->error_message.empty()) {
      c.error_info.message_pos = s;
      c.error_info.message =
          resolve_capture_placeholders(label->rule_->error_message, c);
      c.error_info.label = label->rule_->name;
    }
  }

  // Recovery
  auto len = static_cast<size_t>(-1);
  {
    auto save_log = c.log;
    auto save_reporter = c.error_reporter;
    c.log = nullptr;
    c.error_reporter = nullptr;
    auto se = scope_exit([&]() {
      c.log = save_log;
      c.error_reporter = save_reporter;
    });

    SemanticValues dummy_vs;
    std::any dummy_dt;

    len = rule.parse(s, n, dummy_vs, c, dummy_dt);
  }

  if (success(len)) {
    c.recovered = true;

    if (c.log || c.error_reporter) {
      c.error_info.output_log(c.log, c.error_reporter, c.s, c.l);
      c.error_info.clear();
    }
  }

  // Cut
  if (!c.cut_stack.empty()) {
    c.cut_stack.back() = true;

    if (c.cut_stack.size() == 1) {
      // TODO: Remove unneeded entries in packrat memoise table
    }
  }

  return len;
}

void Sequence::accept(Visitor &v) { v.visit(*this); }
void PrioritizedChoice::accept(Visitor &v) { v.visit(*this); }
void Repetition::accept(Visitor &v) { v.visit(*this); }
void AndPredicate::accept(Visitor &v) { v.visit(*this); }
void NotPredicate::accept(Visitor &v) { v.visit(*this); }
void Dictionary::accept(Visitor &v) { v.visit(*this); }
void LiteralString::accept(Visitor &v) { v.visit(*this); }
void CharacterClass::accept(Visitor &v) { v.visit(*this); }
void Character::accept(Visitor &v) { v.visit(*this); }
void AnyCharacter::accept(Visitor &v) { v.visit(*this); }
void CaptureScope::accept(Visitor &v) { v.visit(*this); }
void Capture::accept(Visitor &v) { v.visit(*this); }
void TokenBoundary::accept(Visitor &v) { v.visit(*this); }
void Ignore::accept(Visitor &v) { v.visit(*this); }
void User::accept(Visitor &v) { v.visit(*this); }
void WeakHolder::accept(Visitor &v) { v.visit(*this); }
void Holder::accept(Visitor &v) { v.visit(*this); }
void Reference::accept(Visitor &v) { v.visit(*this); }
void Whitespace::accept(Visitor &v) { v.visit(*this); }
void BackReference::accept(Visitor &v) { v.visit(*this); }
void PrecedenceClimbing::accept(Visitor &v) { v.visit(*this); }
void Recovery::accept(Visitor &v) { v.visit(*this); }
void Cut::accept(Visitor &v) { v.visit(*this); }

void AssignIDToDefinition::visit(Holder &ope) {
  auto p = static_cast<void *>(ope.outer_);
  if (ids.count(p)) { return; }
  auto id = ids.size();
  ids[p] = id;
  ope.outer_->id = id;
  ope.ope_->accept(*this);
}

void AssignIDToDefinition::visit(Reference &ope) {
  if (ope.rule_) {
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
    ope.rule_->accept(*this);
  }
}

void AssignIDToDefinition::visit(PrecedenceClimbing &ope) {
  ope.atom_->accept(*this);
  ope.binop_->accept(*this);
}

void TokenChecker::visit(Reference &ope) {
  if (ope.is_macro_) {
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  } else {
    has_rule_ = true;
  }
}

void FindLiteralToken::visit(Reference &ope) {
  if (ope.is_macro_) {
    ope.rule_->accept(*this);
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  }
}

void ComputeCanBeEmpty::visit(Reference &ope) {
  result = ope.rule_ && ope.rule_->can_be_empty;
}

void DetectLeftRecursion::visit(Reference &ope) {
  if (ope.name_ == name_) {
    error_s = ope.s_;
  } else if (!ope.rule_ && !macro_args_stack_.empty()) {
    // Macro parameter reference: resolve through nested macro arg
    // stacks (e.g. B(X) <- C(X) where X is itself a param ref).
    auto resolved = resolve_macro_arg(ope.iarg_);
    if (resolved) {
      resolved->accept(*this);
      if (done_ == false) { return; }
    }
  } else if (!refs_.count(ope.name_)) {
    refs_.insert(ope.name_);
    if (ope.rule_) {
      if (ope.is_macro_) { macro_args_stack_.push_back(&ope.args_); }
      ope.rule_->accept(*this);
      if (ope.is_macro_) { macro_args_stack_.pop_back(); }
      if (done_ == false) { return; }
    }
  }
  // If the referenced rule can match empty, don't mark as done —
  // the sequence may continue past this element to find LR.
  if (!ope.rule_ && !macro_args_stack_.empty()) {
    auto resolved = resolve_macro_arg(ope.iarg_);
    if (resolved) {
      ComputeCanBeEmpty cbe;
      resolved->accept(cbe);
      done_ = !cbe.result;
    } else {
      done_ = true;
    }
  } else {
    done_ = !(ope.rule_ && ope.rule_->can_be_empty);
  }
}

std::shared_ptr<Ope>
DetectLeftRecursion::resolve_macro_arg(size_t iarg) const {
  for (int i = static_cast<int>(macro_args_stack_.size()) - 1; i >= 0; i--) {
    auto &args = *macro_args_stack_[i];
    if (iarg >= args.size()) { return nullptr; }
    auto ref = dynamic_cast<Reference *>(args[iarg].get());
    if (ref && !ref->rule_) {
      // Another param ref — resolve using parent level's args
      iarg = ref->iarg_;
      continue;
    }
    return args[iarg];
  }
  return nullptr;
}

void HasEmptyElement::visit(Sequence &ope) {
  auto save_is_empty = false;
  const char *save_error_s = nullptr;
  std::string save_error_name;

  auto it = ope.opes_.begin();
  while (it != ope.opes_.end()) {
    (*it)->accept(*this);
    if (!is_empty) {
      ++it;
      while (it != ope.opes_.end()) {
        DetectInfiniteLoop vis(refs_, has_error_cache_);
        (*it)->accept(vis);
        if (vis.has_error) {
          is_empty = true;
          error_s = vis.error_s;
          error_name = vis.error_name;
        }
        ++it;
      }
      return;
    }

    save_is_empty = is_empty;
    save_error_s = error_s;
    save_error_name = error_name;

    is_empty = false;
    error_name.clear();
    ++it;
  }

  is_empty = save_is_empty;
  error_s = save_error_s;
  error_name = save_error_name;
}

void HasEmptyElement::visit(Reference &ope) {
  auto it = std::find_if(refs_.begin(), refs_.end(),
                         [&](const std::pair<const char *, std::string> &ref) {
                           return ope.name_ == ref.second;
                         });
  if (it != refs_.end()) { return; }

  if (ope.rule_) {
    refs_.emplace_back(ope.s_, ope.name_);
    ope.rule_->accept(*this);
    refs_.pop_back();
  }
}

void DetectInfiniteLoop::visit(Reference &ope) {
  auto it = std::find_if(refs_.begin(), refs_.end(),
                         [&](const std::pair<const char *, std::string> &ref) {
                           return ope.name_ == ref.second;
                         });
  if (it != refs_.end()) { return; }

  if (ope.rule_) {
    auto it = has_error_cache_.find(ope.name_);
    if (it != has_error_cache_.end()) {
      has_error = it->second;
    } else {
      refs_.emplace_back(ope.s_, ope.name_);
      ope.rule_->accept(*this);
      refs_.pop_back();
      has_error_cache_[ope.name_] = has_error;
    }
  }

  if (ope.is_macro_) {
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  }
}

void ReferenceChecker::visit(Reference &ope) {
  auto it = std::find(params_.begin(), params_.end(), ope.name_);
  if (it != params_.end()) { return; }

  if (!grammar_.count(ope.name_)) {
    error_s[ope.name_] = ope.s_;
    error_message[ope.name_] = "'" + ope.name_ + "' is not defined.";
  } else {
    if (!referenced.count(ope.name_)) { referenced.insert(ope.name_); }
    const auto &rule = grammar_.at(ope.name_);
    if (rule.is_macro) {
      if (!ope.is_macro_ || ope.args_.size() != rule.params.size()) {
        error_s[ope.name_] = ope.s_;
        error_message[ope.name_] = "incorrect number of arguments.";
      }
    } else if (ope.is_macro_) {
      error_s[ope.name_] = ope.s_;
      error_message[ope.name_] = "'" + ope.name_ + "' is not macro.";
    }
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  }
}

void ComputeFirstSet::visit(Reference &ope) {
  if (!ope.rule_) {
    // Macro parameter reference — can't predict what it will match
    result_.any_char = true;
    return;
  }

  auto it = cache_.find(ope.rule_);
  FirstSet computed;
  const FirstSet *rule_fs;
  if (it != cache_.end()) {
    rule_fs = &it->second;
  } else {
    if (!refs_.insert(ope.rule_).second) {
      cycle_count_++; // cycle / left recursion
      return;
    }
    auto save = std::exchange(result_, FirstSet{});
    auto saved_cycle_count = cycle_count_;
    ope.rule_->accept(*this);
    computed = std::move(result_);
    result_ = std::move(save);
    refs_.erase(ope.rule_);
    if (cycle_count_ == saved_cycle_count) {
      // Cycle-free: cached value is complete and safe to reuse.
      it = cache_.try_emplace(ope.rule_, std::move(computed)).first;
      rule_fs = &it->second;
    } else {
      // Cycle was hit during this rule's computation — its result may be
      // missing contributions from rules that were on the call stack.
      // Use the value here but do not cache it for other call contexts.
      rule_fs = &computed;
    }
  }

  result_.merge(*rule_fs);
  if (!result_.first_literal) {
    result_.first_literal = rule_fs->first_literal;
  }
  if (!result_.first_rule) {
    result_.first_rule = rule_fs->first_rule
                             ? rule_fs->first_rule
                             : (ope.rule_->is_token() ? ope.rule_ : nullptr);
  }
}

void SetupFirstSets::visit(Reference &ope) {
  if (!ope.rule_) { return; }
  ope.rule_->accept(*this); // re-entry is guarded at the rule's Holder
}

// Guard rule setup by Definition so a SetupFirstSets shared across all rules
// visits each rule's body at most once for the whole grammar. Without this the
// per-rule setup re-walks every reachable rule once per referencing rule, which
// is O(N^2) for grammars with dense cross-references.
void SetupFirstSets::visit(Holder &ope) {
  if (!visited_rules_.insert(ope.outer_).second) { return; }
  ope.ope_->accept(*this);
}

void SetupFirstSets::visit(Sequence &ope) {
  ope.kw_guard_.reset();
  setup_keyword_guarded_identifier(ope);
  for (const auto &op : ope.opes_) {
    op->accept(*this);
  }
}

void SetupFirstSets::setup_keyword_guarded_identifier(Sequence &seq) {
  // Detect pattern: NotPredicate(Reference→PrioritizedChoice<literals>)
  //                 TokenBoundary(Sequence[CharacterClass,
  //                 Repetition(CharacterClass)])
  // This is the pattern used by: PlainIdentifier <- !ReservedKeyword
  // <[a-z_]i[a-z0-9_]i*>
  if (seq.opes_.size() != 2) { return; }

  // Child 0 must be NotPredicate
  auto *not_pred = dynamic_cast<NotPredicate *>(seq.opes_[0].get());
  if (!not_pred) { return; }

  // NotPredicate's child must be Reference to a rule
  auto *ref = dynamic_cast<Reference *>(not_pred->ope_.get());
  if (!ref || !ref->rule_) { return; }

  // The referenced rule's inner operator (Holder) must contain
  // PrioritizedChoice
  auto *holder = dynamic_cast<Holder *>(ref->get_core_operator().get());
  if (!holder) { return; }
  auto *choice = dynamic_cast<PrioritizedChoice *>(holder->ope_.get());
  if (!choice) { return; }

  // Extract keywords from PrioritizedChoice alternatives
  std::vector<std::string> exact_keywords;
  std::vector<std::string> prefix_keywords;

  for (const auto &alt : choice->opes_) {
    auto *lit = dynamic_cast<LiteralString *>(alt.get());
    if (lit) {
      if (!lit->ignore_case_) { return; }
      exact_keywords.push_back(to_lower(lit->lit_));
      continue;
    }
    // Check for compound keyword (Sequence of LiteralStrings)
    auto *sub_seq = dynamic_cast<Sequence *>(alt.get());
    if (sub_seq && !sub_seq->opes_.empty()) {
      auto *first_lit = dynamic_cast<LiteralString *>(sub_seq->opes_[0].get());
      if (first_lit) {
        auto all_ignore_case_lits =
            std::all_of(sub_seq->opes_.begin(), sub_seq->opes_.end(),
                        [](const auto &child) {
                          auto *l = dynamic_cast<LiteralString *>(child.get());
                          return l && l->ignore_case_;
                        });
        if (all_ignore_case_lits) {
          prefix_keywords.push_back(to_lower(first_lit->lit_));
          continue;
        }
      }
    }
    // Unrecognized alternative — bail out
    return;
  }

  if (exact_keywords.empty()) { return; }

  // Child 1 must be TokenBoundary
  auto *tb = dynamic_cast<TokenBoundary *>(seq.opes_[1].get());
  if (!tb) { return; }

  // TokenBoundary content: Sequence[CharacterClass, Repetition(CharacterClass)]
  // or just CharacterClass (single char identifier)
  CharacterClass *first_cc = nullptr;
  CharacterClass *rest_cc = nullptr;

  auto *inner_seq = dynamic_cast<Sequence *>(tb->ope_.get());
  if (inner_seq && inner_seq->opes_.size() == 2) {
    first_cc = dynamic_cast<CharacterClass *>(inner_seq->opes_[0].get());
    auto *rep = dynamic_cast<Repetition *>(inner_seq->opes_[1].get());
    if (rep) { rest_cc = dynamic_cast<CharacterClass *>(rep->ope_.get()); }
  }

  if (!first_cc || !rest_cc) { return; }
  if (!first_cc->is_ascii_only() || !rest_cc->is_ascii_only()) { return; }

  // All conditions met — set up the fast path
  auto kw = std::make_unique<KeywordGuardData>();
  kw->identifier_first = first_cc->ascii_bitset();
  kw->identifier_rest = rest_cc->ascii_bitset();

  // Compute keyword length range for early-out in hot path
  size_t min_len = SIZE_MAX, max_len = 0;
  for (const auto &k : exact_keywords) {
    min_len = std::min(min_len, k.size());
    max_len = std::max(max_len, k.size());
  }
  for (const auto &k : prefix_keywords) {
    min_len = std::min(min_len, k.size());
    max_len = std::max(max_len, k.size());
  }
  kw->min_keyword_len = min_len;
  kw->max_keyword_len = max_len;

  kw->exact_keywords = std::move(exact_keywords);
  kw->prefix_keywords = std::move(prefix_keywords);
  seq.kw_guard_ = std::move(kw);
}

// Compute which rules benefit from packrat memoization.
// A rule benefits if it's reachable from 2+ alternatives of the same
// PrioritizedChoice (backtracking will re-visit it at the same position).
void Definition::initialize_packrat_filter() const {
  std::call_once(packrat_filter_init_, [&]() {
    auto def_count = definition_ids_.size();
    if (def_count == 0) { return; }

    // Collect rule IDs that can be invoked at the *same start position* as
    // the given Ope subtree (leftmost reachability). A packrat cache hit
    // requires the same rule to be queried twice at the same position, and
    // in a PEG that only happens when alternatives of a choice share a
    // leftmost prefix — rules reachable only past a consuming element can
    // never be re-queried by a sibling alternative.
    struct CollectLeftmostRules : public TraversalVisitor {
      using TraversalVisitor::visit;
      std::vector<bool> reachable; // indexed by def_id
      std::vector<bool>
          visited_rules; // indexed by def_id; guards Holder cycles

      CollectLeftmostRules(size_t n)
          : reachable(n, false), visited_rules(n, false) {}

      void visit(Sequence &ope) override {
        // Only elements up to (and including) the first one that must
        // consume input are at the start position.
        for (auto &op : ope.opes_) {
          op->accept(*this);
          ComputeCanBeEmpty empty_vis;
          op->accept(empty_vis);
          if (!empty_vis.result) { break; }
        }
      }
      void visit(Holder &ope) override {
        auto id = ope.outer_->id;
        if (id < reachable.size()) {
          reachable[id] = true;

          // Grammars built directly via the combinator API embed rules through
          // WeakHolder rather than Reference, so a recursive rule forms a
          // Holder cycle with no Reference to break it. Guard re-entry to avoid
          // infinite recursion (reachability is monotone, so revisiting a rule
          // we have already traversed adds nothing).
          if (visited_rules[id]) { return; }
          visited_rules[id] = true;
        }
        ope.ope_->accept(*this);
      }
      void visit(Reference &ope) override {
        if (ope.rule_ && ope.rule_->id < reachable.size() &&
            !reachable[ope.rule_->id]) {
          reachable[ope.rule_->id] = true;
          ope.rule_->accept(*this);
        }
      }
    };

    // Find rules that benefit: leftmost-reachable from 2+ alternatives of
    // the same choice
    std::vector<bool> benefits(def_count, false);

    struct FindBacktrackRules : public TraversalVisitor {
      using TraversalVisitor::visit;
      std::vector<bool> &benefits;
      size_t def_count;
      std::vector<bool> visited_rules; // indexed by def_id

      FindBacktrackRules(std::vector<bool> &b, size_t n)
          : benefits(b), def_count(n), visited_rules(n, false) {}

      void visit(PrioritizedChoice &ope) override {
        // For each alternative, collect leftmost-reachable rules
        std::vector<std::vector<bool>> alt_reachable;
        for (auto &op : ope.opes_) {
          CollectLeftmostRules clr(def_count);
          op->accept(clr);
          alt_reachable.push_back(std::move(clr.reachable));
        }

        // Mark rules leftmost-reachable from 2+ alternatives
        for (size_t id = 0; id < def_count; id++) {
          size_t count = 0;
          for (auto &alt : alt_reachable) {
            if (alt[id]) { count++; }
          }
          if (count >= 2) { benefits[id] = true; }
        }

        // Recurse into alternatives
        for (auto &op : ope.opes_) {
          op->accept(*this);
        }
      }
      void visit(Holder &ope) override {
        auto id = ope.outer_->id;
        if (id < visited_rules.size() && !visited_rules[id]) {
          visited_rules[id] = true;
          ope.ope_->accept(*this);
        }
      }
      void visit(Reference &ope) override {
        if (ope.rule_) { ope.rule_->accept(*this); }
      }
    };

    FindBacktrackRules finder(benefits, def_count);
    holder_->accept(finder);
    if (whitespaceOpe) { whitespaceOpe->accept(finder); }
    if (wordOpe) { wordOpe->accept(finder); }

    // Left-recursive rules read and write the packrat cache directly during
    // seed-growing, so they must stay in the cached set.
    for (const auto &[ptr, id] : definition_ids_) {
      auto *def = static_cast<Definition *>(ptr);
      if (def->is_left_recursive && id < def_count) { benefits[id] = true; }
    }

    // Compact index: def_id -> slot in the cache tables (-1 = guard only)
    packrat_index_.assign(def_count, -1);
    int32_t k = 0;
    for (size_t id = 0; id < def_count; id++) {
      if (benefits[id]) { packrat_index_[id] = k++; }
    }
    packrat_cached_count_ = static_cast<size_t>(k);
  });
}

void LinkReferences::visit(Reference &ope) {
  // Check if the reference is a macro parameter
  auto found_param = false;
  for (size_t i = 0; i < params_.size(); i++) {
    const auto &param = params_[i];
    if (param == ope.name_) {
      ope.iarg_ = i;
      found_param = true;
      break;
    }
  }

  // Check if the reference is a definition rule
  if (!found_param && grammar_.count(ope.name_)) {
    auto &rule = grammar_.at(ope.name_);
    ope.rule_ = &rule;
  }

  for (const auto &arg : ope.args_) {
    arg->accept(*this);
  }
}

void FindReference::visit(Reference &ope) {
  for (size_t i = 0; i < args_.size(); i++) {
    const auto &name = params_[i];
    if (name == ope.name_) {
      found_ope = args_[i];
      return;
    }
  }
  found_ope = ope.shared_from_this();
}

namespace detail {
struct GrammarBlobImpl {
  enum Tag : uint8_t {
    T_Sequence,
    T_Choice,
    T_Repetition,
    T_And,
    T_Not,
    T_Dictionary,
    T_Literal,
    T_CharClass,
    T_Char,
    T_AnyChar,
    T_CaptureScope,
    T_Capture,
    T_TokenBoundary,
    T_Ignore,
    T_BackRef,
    T_Reference,
    T_Whitespace,
    T_Recovery,
    T_Cut,
    T_PrecedenceClimbing,
    T_Null
  };

  struct Writer {
    std::vector<uint8_t> b;
    void u8(uint8_t v) { b.push_back(v); }
    void u32(uint32_t v) {
      for (int i = 0; i < 4; i++)
        b.push_back((v >> (8 * i)) & 0xff);
    }
    void u64(uint64_t v) {
      for (int i = 0; i < 8; i++)
        b.push_back((v >> (8 * i)) & 0xff);
    }
    void str(const std::string &s) {
      u32((uint32_t)s.size());
      b.insert(b.end(), s.begin(), s.end());
    }
  };

  static void write_ope(Writer &w, const std::shared_ptr<Ope> &o) {
    if (!o) {
      w.u8(T_Null);
      return;
    }
    Ope *p = o.get();
    if (auto x = dynamic_cast<Sequence *>(p)) {
      w.u8(T_Sequence);
      w.u32((uint32_t)x->opes_.size());
      for (auto &c : x->opes_)
        write_ope(w, c);
    } else if (auto x = dynamic_cast<PrioritizedChoice *>(p)) {
      w.u8(T_Choice);
      w.u8(x->for_label_ ? 1 : 0);
      w.u32((uint32_t)x->opes_.size());
      for (auto &c : x->opes_)
        write_ope(w, c);
    } else if (auto x = dynamic_cast<Repetition *>(p)) {
      w.u8(T_Repetition);
      w.u64(x->min_);
      w.u64(x->max_);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<AndPredicate *>(p)) {
      w.u8(T_And);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<NotPredicate *>(p)) {
      w.u8(T_Not);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Dictionary *>(p)) {
      w.u8(T_Dictionary);
      w.u8(x->trie_.ignore_case_ ? 1 : 0);
      // Recover words in their original choice-index order. The Trie stores
      // each full word's id (its index in the constructor vector), which
      // parse_core reports as vs.choice(). Iterating dic_ directly yields
      // sorted key order and would renumber the choices, so place each word at
      // its id.
      std::vector<std::string> words(x->trie_.items_count());
      for (auto &kv : x->trie_.dic_)
        if (kv.second.match && kv.second.id < words.size())
          words[kv.second.id] = kv.first;
      w.u32((uint32_t)words.size());
      for (auto &s : words)
        w.str(s);
    } else if (auto x = dynamic_cast<LiteralString *>(p)) {
      w.u8(T_Literal);
      w.u8(x->ignore_case_ ? 1 : 0);
      w.str(x->lit_);
    } else if (auto x = dynamic_cast<CharacterClass *>(p)) {
      w.u8(T_CharClass);
      w.u8(x->negated_ ? 1 : 0);
      w.u8(x->ignore_case_ ? 1 : 0);
      w.u32((uint32_t)x->ranges_.size());
      for (auto &r : x->ranges_) {
        w.u32((uint32_t)r.first);
        w.u32((uint32_t)r.second);
      }
    } else if (auto x = dynamic_cast<Character *>(p)) {
      w.u8(T_Char);
      w.u32((uint32_t)x->ch_);
    } else if (dynamic_cast<AnyCharacter *>(p)) {
      w.u8(T_AnyChar);
    } else if (auto x = dynamic_cast<CaptureScope *>(p)) {
      w.u8(T_CaptureScope);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Capture *>(p)) {
      if (x->match_action_) {
        throw std::runtime_error(
            "GrammarBlob: Capture with a match action is not serializable");
      }
      w.u8(T_Capture);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<TokenBoundary *>(p)) {
      w.u8(T_TokenBoundary);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Ignore *>(p)) {
      w.u8(T_Ignore);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<BackReference *>(p)) {
      w.u8(T_BackRef);
      w.str(x->name_);
    } else if (auto x = dynamic_cast<Reference *>(p)) {
      w.u8(T_Reference);
      w.u8(x->is_macro_ ? 1 : 0);
      w.str(x->name_);
      w.u32((uint32_t)x->args_.size());
      for (auto &a : x->args_)
        write_ope(w, a);
    } else if (auto x = dynamic_cast<Whitespace *>(p)) {
      w.u8(T_Whitespace);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Recovery *>(p)) {
      w.u8(T_Recovery);
      write_ope(w, x->ope_);
    } else if (dynamic_cast<Cut *>(p)) {
      w.u8(T_Cut);
    } else if (auto x = dynamic_cast<PrecedenceClimbing *>(p)) {
      w.u8(T_PrecedenceClimbing);
      write_ope(w, x->atom_);
      write_ope(w, x->binop_);
      w.u32((uint32_t)x->info_.size());
      for (auto &[key, pri] : x->info_) {
        w.str(std::string(key));
        w.u64((uint64_t)pri.first);
        w.u8((uint8_t)pri.second);
      }
    } else {
      throw std::runtime_error(
          "GrammarBlob: operator not serializable (a custom User operator or "
          "a Capture with a match action)");
    }
  }

  struct Reader {
    const uint8_t *p, *end;
    uint8_t u8() {
      if (p >= end)
        throw std::runtime_error("GrammarBlob: unexpected end of blob");
      return *p++;
    }
    uint32_t u32() {
      uint32_t v = 0;
      for (int i = 0; i < 4; i++)
        v |= (uint32_t)u8() << (8 * i);
      return v;
    }
    uint64_t u64() {
      uint64_t v = 0;
      for (int i = 0; i < 8; i++)
        v |= (uint64_t)u8() << (8 * i);
      return v;
    }
    std::string str() {
      uint32_t n = u32();
      std::string s((const char *)p, (const char *)p + n);
      p += n;
      return s;
    }
  };

  static std::shared_ptr<Ope> read_ope(Reader &r, Grammar &g,
                                       Definition *owner) {
    switch (r.u8()) {
    case T_Null: return nullptr;
    case T_Sequence: {
      uint32_t n = r.u32();
      std::vector<std::shared_ptr<Ope>> v;
      for (uint32_t i = 0; i < n; i++)
        v.push_back(read_ope(r, g, owner));
      return std::make_shared<Sequence>(std::move(v));
    }
    case T_Choice: {
      bool fl = r.u8();
      uint32_t n = r.u32();
      std::vector<std::shared_ptr<Ope>> v;
      for (uint32_t i = 0; i < n; i++)
        v.push_back(read_ope(r, g, owner));
      auto c = std::make_shared<PrioritizedChoice>(std::move(v));
      c->for_label_ = fl;
      return c;
    }
    case T_Repetition: {
      uint64_t mn = r.u64(), mx = r.u64();
      auto o = read_ope(r, g, owner);
      return std::make_shared<Repetition>(o, mn, mx);
    }
    case T_And: return std::make_shared<AndPredicate>(read_ope(r, g, owner));
    case T_Not: return std::make_shared<NotPredicate>(read_ope(r, g, owner));
    case T_Dictionary: {
      bool ic = r.u8();
      uint32_t n = r.u32();
      std::vector<std::string> words;
      for (uint32_t i = 0; i < n; i++)
        words.push_back(r.str());
      return std::make_shared<Dictionary>(words, ic);
    }
    case T_Literal: {
      bool ic = r.u8();
      std::string s = r.str();
      return std::make_shared<LiteralString>(std::move(s), ic);
    }
    case T_CharClass: {
      bool neg = r.u8(), ic = r.u8();
      uint32_t n = r.u32();
      std::vector<std::pair<char32_t, char32_t>> ranges;
      for (uint32_t i = 0; i < n; i++) {
        auto lo = r.u32(), hi = r.u32();
        ranges.emplace_back((char32_t)lo, (char32_t)hi);
      }
      return std::make_shared<CharacterClass>(ranges, neg, ic);
    }
    case T_Char: return std::make_shared<Character>((char32_t)r.u32());
    case T_AnyChar: return std::make_shared<AnyCharacter>();
    case T_CaptureScope:
      return std::make_shared<CaptureScope>(read_ope(r, g, owner));
    case T_Capture: {
      auto o = read_ope(r, g, owner);
      return std::make_shared<Capture>(o, nullptr);
    }
    case T_TokenBoundary:
      return std::make_shared<TokenBoundary>(read_ope(r, g, owner));
    case T_Ignore: return std::make_shared<Ignore>(read_ope(r, g, owner));
    case T_BackRef: return std::make_shared<BackReference>(r.str());
    case T_Reference: {
      bool im = r.u8();
      std::string nm = r.str();
      uint32_t n = r.u32();
      std::vector<std::shared_ptr<Ope>> args;
      for (uint32_t i = 0; i < n; i++)
        args.push_back(read_ope(r, g, owner));
      return std::make_shared<Reference>(g, nm, nullptr, im, args);
    }
    case T_Whitespace:
      return std::make_shared<Whitespace>(read_ope(r, g, owner));
    case T_Recovery: return std::make_shared<Recovery>(read_ope(r, g, owner));
    case T_Cut: return std::make_shared<Cut>();
    case T_PrecedenceClimbing: {
      if (!owner) {
        throw std::runtime_error(
            "GrammarBlob: 'precedence' operator outside a rule body");
      }
      auto atom = read_ope(r, g, owner);
      auto binop = read_ope(r, g, owner);
      uint32_t n = r.u32();
      auto pc = std::make_shared<PrecedenceClimbing>(
          atom, binop, PrecedenceClimbing::BinOpeInfo{}, *owner);
      // info_ keys are string_views; back them with owned strings whose
      // addresses stay stable (reserve avoids reallocation, and the node is
      // never moved once held by shared_ptr).
      pc->info_keys_.reserve(n);
      for (uint32_t i = 0; i < n; i++) {
        std::string key = r.str();
        auto level = (size_t)r.u64();
        auto assoc = (char)r.u8();
        pc->info_keys_.push_back(std::move(key));
        pc->info_[pc->info_keys_.back()] = std::pair(level, assoc);
      }
      return pc;
    }
    default: throw std::runtime_error("GrammarBlob: bad operator tag");
    }
  }

  static const uint32_t MAGIC = 0x50454732; // "PEG2"

  static std::vector<uint8_t> serialize(const Grammar &g,
                                        const std::string &start) {
    Writer w;
    w.u32(MAGIC);
    w.str(start);
    w.u32((uint32_t)g.size());
    for (auto &[name, def] : g) {
      w.str(name);
      uint8_t flags =
          (def.ignoreSemanticValue ? 1 : 0) | (def.is_macro ? 2 : 0) |
          (def.no_ast_opt ? 4 : 0) | (def.eoi_check ? 8 : 0) |
          (def.enablePackratParsing ? 16 : 0) |
          (def.is_left_recursive ? 32 : 0) | (def.can_be_empty ? 64 : 0) |
          (def.disable_action ? 128 : 0);
      w.u8(flags);
      uint8_t flags2 = (def.no_whitespace ? 1 : 0);
      w.u8(flags2);
      w.u32((uint32_t)def.params.size());
      for (auto &s : def.params)
        w.str(s);
      w.str(def.ast_name);
      w.str(def.error_message);
      write_ope(w, const_cast<Definition &>(def).get_core_operator());
    }
    return std::move(w.b);
  }

  static std::shared_ptr<Grammar> deserialize(const std::vector<uint8_t> &blob,
                                              std::string &start_out) {
    Reader r{blob.data(), blob.data() + blob.size()};
    if (r.u32() != MAGIC)
      throw std::runtime_error("GrammarBlob: bad magic / not a grammar blob");
    start_out = r.str();
    uint32_t ndef = r.u32();
    auto g = std::make_shared<Grammar>();
    // Create each Definition before reading its body: a PrecedenceClimbing node
    // needs a stable reference to its owning rule at construction. Grammar is a
    // node-based map, so references stay valid as later rules are inserted.
    for (uint32_t i = 0; i < ndef; i++) {
      std::string name = r.str();
      uint8_t flags = r.u8();
      uint8_t flags2 = r.u8();
      uint32_t np = r.u32();
      std::vector<std::string> params;
      for (uint32_t k = 0; k < np; k++)
        params.push_back(r.str());
      std::string ast_name = r.str();
      std::string err = r.str();

      auto &def = (*g)[name];
      def.name = name;
      def.ignoreSemanticValue = flags & 1;
      def.is_macro = flags & 2;
      def.no_ast_opt = flags & 4;
      def.eoi_check = flags & 8;
      def.enablePackratParsing = flags & 16;
      def.is_left_recursive = flags & 32;
      def.can_be_empty = flags & 64;
      def.disable_action = flags & 128;
      def.no_whitespace = flags2 & 1;
      def.params = std::move(params);
      def.ast_name = std::move(ast_name);
      def.error_message = std::move(err);

      auto body = read_ope(r, *g, &def);
      def <= body;
    }
    for (auto &x : *g) {
      LinkReferences vis(*g, x.second.params);
      x.second.accept(vis);
      // TraversalVisitor descends only into a PrecedenceClimbing's atom_. In
      // the from-source path binop_ is linked while the body is still a
      // Sequence, before precedence lowering; a deserialized node is built
      // already lowered so its binop_ reference must be linked explicitly here.
      auto core = x.second.get_core_operator();
      if (auto pc = std::dynamic_pointer_cast<PrecedenceClimbing>(core)) {
        pc->binop_->accept(vis);
      }
    }
    {
      SetupFirstSets vis; // shared across rules -> O(N)
      for (auto &x : *g)
        x.second.accept(vis);
    }
    // Re-derive automatic whitespace/word skipping on the start rule from the
    // %whitespace / %word definitions, exactly as ParserGenerator does. Sharing
    // the (already linked and first-set) definition operators avoids leaving
    // references inside the skipping ope unlinked, and keeps the blob smaller.
    if (g->count(WHITESPACE_DEFINITION_NAME)) {
      (*g)[start_out].whitespaceOpe =
          wsp((*g)[WHITESPACE_DEFINITION_NAME].get_core_operator());
    }
    if (g->count(WORD_DEFINITION_NAME)) {
      (*g)[start_out].wordOpe = (*g)[WORD_DEFINITION_NAME].get_core_operator();
    }
    return g;
  }
};
} // namespace detail



std::vector<uint8_t> GrammarBlob::serialize(const Grammar &g,
                                            const std::string &start) {
  return detail::GrammarBlobImpl::serialize(g, start);
}

std::shared_ptr<Grammar> GrammarBlob::deserialize(
    const std::vector<uint8_t> &blob, std::string &start_out) {
  return detail::GrammarBlobImpl::deserialize(blob, start_out);
}

/*-----------------------------------------------------------------------------
 *  PEG parser generator
 *---------------------------------------------------------------------------*/

class ParserGenerator {
public:
  struct ParserContext {
    std::shared_ptr<Grammar> grammar;
    std::string start;
    bool enablePackratParsing = false;
  };

  static ParserContext parse(const char *s, size_t n, const Rules &rules,
                             Log log, std::string_view start,
                             bool enable_left_recursion = true) {
    return get_instance().perform_core(s, n, rules, log, std::string(start),
                                       enable_left_recursion);
  }

  // For debugging purpose
  static bool parse_test(const char *d, const char *s) {
    Data data;
    std::any dt = &data;

    auto n = strlen(s);
    auto r = get_instance().g[d].parse(s, n, dt);
    return r.ret && r.len == n;
  }

#if defined(__cpp_lib_char8_t)
  static bool parse_test(const char *d, const char8_t *s) {
    return parse_test(d, reinterpret_cast<const char *>(s));
  }
#endif

private:
  static ParserGenerator &get_instance() {
    static ParserGenerator instance;
    return instance;
  }

  ParserGenerator() {
    make_grammar();
    setup_actions();
    // Apply First-Set filtering to the bootstrap meta-grammar itself so that
    // parsing a grammar (the bulk of load_grammar) skips alternatives whose
    // next byte cannot match. This is safe -- First-Set filtering only skips
    // alternatives that would have failed anyway, so no semantic action that
    // would have committed is skipped (unlike packrat, which is unsound here).
    {
      SetupFirstSets vis;
      for (auto &x : g) {
        x.second.accept(vis);
      }
    }
  }

  struct Instruction {
    std::string type;
    std::any data;
    std::string_view sv;
  };

  struct Data {
    std::shared_ptr<Grammar> grammar;
    std::string start;
    const char *start_pos = nullptr;

    std::vector<std::pair<std::string, const char *>> duplicates_of_definition;

    std::vector<std::pair<std::string, const char *>> duplicates_of_instruction;
    std::map<std::string, std::vector<Instruction>> instructions;

    std::vector<std::pair<std::string, const char *>> undefined_back_references;
    std::vector<std::set<std::string_view>> captures_stack{{}};

    std::set<std::string_view> captures_in_current_definition;
    bool enablePackratParsing = true;

    Data() : grammar(std::make_shared<Grammar>()) {}
  };

  class SyntaxErrorException : public std::runtime_error {
  public:
    SyntaxErrorException(const char *what_arg, std::pair<size_t, size_t> r)
        : std::runtime_error(what_arg), r_(r) {}

    std::pair<size_t, size_t> line_info() const { return r_; }

  private:
    std::pair<size_t, size_t> r_;
  };

  void make_grammar() {
    // Setup PEG syntax parser
    g["Grammar"] <= seq(g["Spacing"], oom(g["Definition"]), g["EndOfFile"]);
    // Left-factored: parse the rule name (IdentCont) once, then optionally the
    // macro parameter list. `opt(Parameters)` pushes a value only for a macro
    // (so the value layout matches the old two-alternative form), and Spacing
    // (~, no value) consumes the gap before LEFTARROW that Identifier used to.
    g["Definition"] <= seq(g["Ignore"], g["IdentCont"], opt(g["Parameters"]),
                           g["Spacing"], g["LEFTARROW"], g["Expression"],
                           opt(g["Instruction"]));
    g["Expression"] <= seq(g["Sequence"], zom(seq(g["SLASH"], g["Sequence"])));
    g["Sequence"] <= zom(cho(g["CUT"], g["Prefix"]));
    g["Prefix"] <= seq(opt(cho(g["AND"], g["NOT"])), g["SuffixWithLabel"]);
    g["SuffixWithLabel"] <=
        seq(g["Suffix"], opt(seq(g["LABEL"], g["Identifier"])));
    g["Suffix"] <= seq(g["Primary"], opt(g["Loop"]));
    g["Loop"] <= cho(g["QUESTION"], g["STAR"], g["PLUS"], g["Repetition"]);
    // Left-factored: a macro reference (`Name(args)`) and a plain reference
    // (`Name`) share the leading `Ignore IdentCont`, so parse it once and let
    // `opt(Arguments)` decide. opt() pushes the argument list only for a macro
    // reference, so vs.size() distinguishes the two in the action.
    g["Primary"] <=
        cho(seq(g["Ignore"], g["IdentCont"], opt(g["Arguments"]), g["Spacing"],
                npd(seq(opt(g["Parameters"]), g["LEFTARROW"]))),
            seq(g["OPEN"], g["Expression"], g["CLOSE"]),
            seq(g["BeginTok"], g["Expression"], g["EndTok"]), g["CapScope"],
            seq(g["BeginCap"], g["Expression"], g["EndCap"]), g["BackRef"],
            g["DictionaryI"], g["LiteralI"], g["Dictionary"], g["Literal"],
            g["NegatedClassI"], g["NegatedClass"], g["ClassI"], g["Class"],
            g["DOT"]);

    g["Identifier"] <= seq(g["IdentCont"], g["Spacing"]);
    g["IdentCont"] <= tok(seq(g["IdentStart"], zom(g["IdentRest"])));

    const static std::vector<std::pair<char32_t, char32_t>> range = {
        {0x0080, 0xFFFF}};
    g["IdentStart"] <= seq(npd(lit(u8(u8"↑"))), npd(lit(u8(u8"⇑"))),
                           cho(cls("a-zA-Z_%"), cls(range)));

    g["IdentRest"] <= cho(g["IdentStart"], cls("0-9"));

    g["Dictionary"] <= seq(g["LiteralD"], oom(seq(g["PIPE"], g["LiteralD"])));

    g["DictionaryI"] <=
        seq(g["LiteralID"], oom(seq(g["PIPE"], g["LiteralID"])));

    auto lit_ope = cho(seq(cls("'"), tok(zom(seq(npd(cls("'")), g["Char"]))),
                           cls("'"), g["Spacing"]),
                       seq(cls("\""), tok(zom(seq(npd(cls("\"")), g["Char"]))),
                           cls("\""), g["Spacing"]));
    g["Literal"] <= lit_ope;
    g["LiteralD"] <= lit_ope;

    auto lit_case_ignore_ope =
        cho(seq(cls("'"), tok(zom(seq(npd(cls("'")), g["Char"]))), lit("'i"),
                g["Spacing"]),
            seq(cls("\""), tok(zom(seq(npd(cls("\"")), g["Char"]))), lit("\"i"),
                g["Spacing"]));
    g["LiteralI"] <= lit_case_ignore_ope;
    g["LiteralID"] <= lit_case_ignore_ope;

    // NOTE: The original Brian Ford's paper uses 'zom' instead of 'oom'.
    g["Class"] <= seq(chr('['), npd(chr('^')),
                      tok(oom(seq(npd(chr(']')), g["Range"]))), chr(']'),
                      g["Spacing"]);
    g["ClassI"] <= seq(chr('['), npd(chr('^')),
                       tok(oom(seq(npd(chr(']')), g["Range"]))), lit("]i"),
                       g["Spacing"]);

    g["NegatedClass"] <= seq(lit("[^"),
                             tok(oom(seq(npd(chr(']')), g["Range"]))), chr(']'),
                             g["Spacing"]);
    g["NegatedClassI"] <= seq(lit("[^"),
                              tok(oom(seq(npd(chr(']')), g["Range"]))),
                              lit("]i"), g["Spacing"]);

    // NOTE: This is different from The original Brian Ford's paper, and this
    // modification allows us to specify `[+-]` as a valid char class.
    g["Range"] <= cho(seq(g["Char"], chr('-'), npd(chr(']')), g["Char"]),
                      g["ClassEscape"], g["PosixClass"], g["Char"]);

    g["ClassEscape"] <= seq(chr('\\'), cls("dDwWsS"));
    g["PosixClass"] <=
        seq(lit("[:"), opt(chr('^')), oom(cls("a-z")), lit(":]"));

    g["Char"] <=
        cho(seq(chr('\\'), cls("fnrtv'\"[]\\^-")),
            seq(chr('\\'), cls("0-3"), cls("0-7"), cls("0-7")),
            seq(chr('\\'), cls("0-7"), opt(cls("0-7"))),
            seq(lit("\\x"), cls("0-9a-fA-F"), opt(cls("0-9a-fA-F"))),
            seq(lit("\\u"),
                cho(seq(cho(seq(chr('0'), cls("0-9a-fA-F")), lit("10")),
                        rep(cls("0-9a-fA-F"), 4, 4)),
                    rep(cls("0-9a-fA-F"), 4, 5))),
            seq(npd(chr('\\')), dot()));

    g["Repetition"] <=
        seq(g["BeginBracket"], g["RepetitionRange"], g["EndBracket"]);
    g["RepetitionRange"] <= cho(seq(g["Number"], g["COMMA"], g["Number"]),
                                seq(g["Number"], g["COMMA"]), g["Number"],
                                seq(g["COMMA"], g["Number"]));
    g["Number"] <= seq(oom(cls("0-9")), g["Spacing"]);

    g["CapScope"] <= seq(g["BeginCapScope"], g["Expression"], g["EndCapScope"]);

    g["LEFTARROW"] <= seq(cho(lit("<-"), lit(u8(u8"←"))), g["Spacing"]);
    ~g["SLASH"] <= seq(chr('/'), g["Spacing"]);
    ~g["PIPE"] <= seq(chr('|'), g["Spacing"]);
    g["AND"] <= seq(chr('&'), g["Spacing"]);
    g["NOT"] <= seq(chr('!'), g["Spacing"]);
    g["QUESTION"] <= seq(chr('?'), g["Spacing"]);
    g["STAR"] <= seq(chr('*'), g["Spacing"]);
    g["PLUS"] <= seq(chr('+'), g["Spacing"]);
    ~g["OPEN"] <= seq(chr('('), g["Spacing"]);
    ~g["CLOSE"] <= seq(chr(')'), g["Spacing"]);
    g["DOT"] <= seq(chr('.'), g["Spacing"]);

    g["CUT"] <= seq(lit(u8(u8"↑")), g["Spacing"]);
    ~g["LABEL"] <= seq(cho(chr('^'), lit(u8(u8"⇑"))), g["Spacing"]);

    ~g["Spacing"] <= zom(cho(g["Space"], g["Comment"]));
    g["Comment"] <= seq(chr('#'), zom(seq(npd(g["EndOfLine"]), dot())),
                        opt(g["EndOfLine"]));
    g["Space"] <= cho(chr(' '), chr('\t'), g["EndOfLine"]);
    g["EndOfLine"] <= cho(lit("\r\n"), chr('\n'), chr('\r'));
    g["EndOfFile"] <= npd(dot());

    ~g["BeginTok"] <= seq(chr('<'), g["Spacing"]);
    ~g["EndTok"] <= seq(chr('>'), g["Spacing"]);

    ~g["BeginCapScope"] <= seq(chr('$'), chr('('), g["Spacing"]);
    ~g["EndCapScope"] <= seq(chr(')'), g["Spacing"]);

    g["BeginCap"] <= seq(chr('$'), tok(g["IdentCont"]), chr('<'), g["Spacing"]);
    ~g["EndCap"] <= seq(chr('>'), g["Spacing"]);

    g["BackRef"] <= seq(chr('$'), tok(g["IdentCont"]), g["Spacing"]);

    g["IGNORE"] <= chr('~');

    g["Ignore"] <= opt(g["IGNORE"]);
    g["Parameters"] <= seq(g["OPEN"], g["Identifier"],
                           zom(seq(g["COMMA"], g["Identifier"])), g["CLOSE"]);
    g["Arguments"] <= seq(g["OPEN"], g["Expression"],
                          zom(seq(g["COMMA"], g["Expression"])), g["CLOSE"]);
    ~g["COMMA"] <= seq(chr(','), g["Spacing"]);

    // Instruction grammars
    g["Instruction"] <=
        seq(g["BeginBracket"],
            opt(seq(g["InstructionItem"], zom(seq(g["InstructionItemSeparator"],
                                                  g["InstructionItem"])))),
            g["EndBracket"]);
    g["InstructionItem"] <= cho(g["PrecedenceClimbing"], g["ErrorMessage"],
                                g["NoAstOpt"], g["NoWhitespace"], g["AstName"]);
    ~g["InstructionItemSeparator"] <= seq(chr(';'), g["Spacing"]);

    ~g["SpacesZom"] <= zom(g["Space"]);
    ~g["SpacesOom"] <= oom(g["Space"]);
    ~g["BeginBracket"] <= seq(chr('{'), g["Spacing"]);
    ~g["EndBracket"] <= seq(chr('}'), g["Spacing"]);

    // PrecedenceClimbing instruction
    g["PrecedenceClimbing"] <=
        seq(lit("precedence"), g["SpacesOom"], g["PrecedenceInfo"],
            zom(seq(g["SpacesOom"], g["PrecedenceInfo"])), g["SpacesZom"]);
    g["PrecedenceInfo"] <=
        seq(g["PrecedenceAssoc"],
            oom(seq(ign(g["SpacesOom"]), g["PrecedenceOpe"])));
    g["PrecedenceOpe"] <=
        cho(seq(cls("'"),
                tok(zom(seq(npd(cho(g["Space"], cls("'"))), g["Char"]))),
                cls("'")),
            seq(cls("\""),
                tok(zom(seq(npd(cho(g["Space"], cls("\""))), g["Char"]))),
                cls("\"")),
            tok(oom(seq(npd(cho(g["PrecedenceAssoc"], g["Space"], chr('}'))),
                        dot()))));
    g["PrecedenceAssoc"] <= cls("LR");

    // Error message instruction
    g["ErrorMessage"] <= seq(lit("error_message"), g["SpacesOom"],
                             g["LiteralD"], g["SpacesZom"]);

    // No Ast node optimization instruction
    g["NoAstOpt"] <= seq(lit("no_ast_opt"), g["SpacesZom"]);

    // No whitespace skipping instruction
    g["NoWhitespace"] <= seq(lit("no_whitespace"), g["SpacesZom"]);

    // AST node name override instruction: `{ ast_name: NodeTag }`
    g["AstName"] <= seq(lit("ast_name"), g["SpacesZom"], lit(":"),
                        g["SpacesZom"], g["Identifier"], g["SpacesZom"]);

    // Set definition names
    for (auto &x : g) {
      x.second.name = x.first;
    }
  }

  void setup_actions() {
    g["Definition"] = [&](const SemanticValues &vs, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);

      // Macro iff the optional Parameters matched: its value (the parameter
      // name list) then sits at vs[2]. A plain definition has LEFTARROW's value
      // there instead.
      auto is_macro = vs[2].type() == typeid(std::vector<std::string>);
      auto ignore = std::any_cast<bool>(vs[0]);
      auto name = std::any_cast<std::string>(vs[1]);

      std::vector<std::string> params;
      std::shared_ptr<Ope> ope;
      auto has_instructions = false;

      if (is_macro) {
        params = std::any_cast<std::vector<std::string>>(vs[2]);
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[4]);
        if (vs.size() == 6) { has_instructions = true; }
      } else {
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[3]);
        if (vs.size() == 5) { has_instructions = true; }
      }

      if (has_instructions) {
        auto index = is_macro ? 5 : 4;
        std::unordered_set<std::string> types;
        for (const auto &instruction :
             std::any_cast<std::vector<Instruction>>(vs[index])) {
          const auto &type = instruction.type;
          if (types.find(type) == types.end()) {
            data.instructions[name].push_back(instruction);
            types.insert(instruction.type);
          } else {
            data.duplicates_of_instruction.emplace_back(type,
                                                        instruction.sv.data());
          }
        }
      }

      auto &grammar = *data.grammar;
      if (!grammar.count(name)) {
        auto &rule = grammar[name];
        rule <= ope;
        rule.name = name;
        rule.s_ = vs.sv().data();
        rule.line_ = line_info(vs.ss, rule.s_);
        rule.ignoreSemanticValue = ignore;
        rule.is_macro = is_macro;
        rule.params = params;

        // Reserved `%`-prefixed rules (%whitespace, %word, ...) are directives,
        // not parseable entry points, so they must not become the start rule.
        if (data.start.empty() && name[0] != '%') {
          data.start = rule.name;
          data.start_pos = rule.s_;
        }
      } else {
        data.duplicates_of_definition.emplace_back(name, vs.sv().data());
      }
    };

    g["Definition"].enter = [](const Context & /*c*/, const char * /*s*/,
                               size_t /*n*/, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);
      data.captures_in_current_definition.clear();
    };

    g["Expression"] = [&](const SemanticValues &vs) {
      if (vs.size() == 1) {
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      } else {
        std::vector<std::shared_ptr<Ope>> opes;
        for (auto i = 0u; i < vs.size(); i++) {
          opes.emplace_back(std::any_cast<std::shared_ptr<Ope>>(vs[i]));
        }
        const std::shared_ptr<Ope> ope =
            std::make_shared<PrioritizedChoice>(opes);
        return ope;
      }
    };

    g["Sequence"] = [&](const SemanticValues &vs) {
      if (vs.empty()) {
        return npd(lit(""));
      } else if (vs.size() == 1) {
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      } else {
        std::vector<std::shared_ptr<Ope>> opes;
        for (const auto &x : vs) {
          opes.emplace_back(std::any_cast<std::shared_ptr<Ope>>(x));
        }
        const std::shared_ptr<Ope> ope = std::make_shared<Sequence>(opes);
        return ope;
      }
    };

    g["Prefix"] = [&](const SemanticValues &vs) {
      std::shared_ptr<Ope> ope;
      if (vs.size() == 1) {
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      } else {
        assert(vs.size() == 2);
        auto tok = std::any_cast<char>(vs[0]);
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[1]);
        if (tok == '&') {
          ope = apd(ope);
        } else { // '!'
          ope = npd(ope);
        }
      }
      return ope;
    };

    g["SuffixWithLabel"] = [&](const SemanticValues &vs, std::any &dt) {
      auto ope = std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      if (vs.size() == 1) {
        return ope;
      } else {
        assert(vs.size() == 2);
        auto &data = *std::any_cast<Data *>(dt);
        const auto &ident = std::any_cast<std::string>(vs[1]);
        auto label = ref(*data.grammar, ident, vs.sv().data(), false, {});
        auto recovery = rec(ref(*data.grammar, RECOVER_DEFINITION_NAME,
                                vs.sv().data(), true, {label}));
        return cho4label_(ope, recovery);
      }
    };

    struct Loop {
      enum class Type { opt = 0, zom, oom, rep };
      Type type;
      std::pair<size_t, size_t> range;
    };

    g["Suffix"] = [&](const SemanticValues &vs) {
      auto ope = std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      if (vs.size() == 1) {
        return ope;
      } else {
        assert(vs.size() == 2);
        auto loop = std::any_cast<Loop>(vs[1]);
        switch (loop.type) {
        case Loop::Type::opt: return opt(ope);
        case Loop::Type::zom: return zom(ope);
        case Loop::Type::oom: return oom(ope);
        default: // Regex-like repetition
          return rep(ope, loop.range.first, loop.range.second);
        }
      }
    };

    g["Loop"] = [&](const SemanticValues &vs) {
      switch (vs.choice()) {
      case 0: // Option
        return Loop{Loop::Type::opt, std::pair<size_t, size_t>()};
      case 1: // Zero or More
        return Loop{Loop::Type::zom, std::pair<size_t, size_t>()};
      case 2: // One or More
        return Loop{Loop::Type::oom, std::pair<size_t, size_t>()};
      default: // Regex-like repetition
        return Loop{Loop::Type::rep,
                    std::any_cast<std::pair<size_t, size_t>>(vs[0])};
      }
    };

    g["Primary"] = [&](const SemanticValues &vs, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);

      switch (vs.choice()) {
      case 0: { // Reference / Macro reference (left-factored)
        // Macro reference iff opt(Arguments) matched and pushed the arg list.
        auto is_macro = vs.size() > 2;
        auto ignore = std::any_cast<bool>(vs[0]);
        const auto &ident = std::any_cast<std::string>(vs[1]);

        std::vector<std::shared_ptr<Ope>> args;
        if (is_macro) {
          args = std::any_cast<std::vector<std::shared_ptr<Ope>>>(vs[2]);
        }

        auto ope = ref(*data.grammar, ident, vs.sv().data(), is_macro, args);
        if (ident == RECOVER_DEFINITION_NAME) { ope = rec(ope); }

        if (ignore) {
          return ign(ope);
        } else {
          return ope;
        }
      }
      case 1: { // (Expression)
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      }
      case 2: { // TokenBoundary
        return tok(std::any_cast<std::shared_ptr<Ope>>(vs[0]));
      }
      case 3: { // CaptureScope
        return csc(std::any_cast<std::shared_ptr<Ope>>(vs[0]));
      }
      case 4: { // Capture
        const auto &name = std::any_cast<std::string_view>(vs[0]);
        auto ope = std::any_cast<std::shared_ptr<Ope>>(vs[1]);

        data.captures_stack.back().insert(name);
        data.captures_in_current_definition.insert(name);

        return cap(ope, [name](const char *a_s, size_t a_n, Context &c) {
          c.capture_entries.emplace_back(name, std::string(a_s, a_n));
        });
      }
      default: {
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      }
      }
    };

    g["IdentCont"] = [](const SemanticValues &vs) {
      return std::string(vs.sv().data(), vs.sv().length());
    };

    g["Dictionary"] = [](const SemanticValues &vs) {
      auto items = vs.transform<std::string>();
      return dic(items, false);
    };
    g["DictionaryI"] = [](const SemanticValues &vs) {
      auto items = vs.transform<std::string>();
      return dic(items, true);
    };

    g["Literal"] = [](const SemanticValues &vs) {
      const auto &tok = vs.tokens.front();
      return lit(resolve_escape_sequence(tok.data(), tok.size()));
    };
    g["LiteralI"] = [](const SemanticValues &vs) {
      const auto &tok = vs.tokens.front();
      return liti(resolve_escape_sequence(tok.data(), tok.size()));
    };
    g["LiteralD"] = [](const SemanticValues &vs) {
      auto &tok = vs.tokens.front();
      return resolve_escape_sequence(tok.data(), tok.size());
    };
    g["LiteralID"] = [](const SemanticValues &vs) {
      auto &tok = vs.tokens.front();
      return resolve_escape_sequence(tok.data(), tok.size());
    };

    // A Range produces either a single range (std::pair) or a range list
    // (std::vector<std::pair>) for `\d`-style escapes and POSIX classes.
    auto collect_ranges = [](const SemanticValues &vs) {
      std::vector<std::pair<char32_t, char32_t>> ranges;
      for (const auto &v : vs) {
        if (v.type() == typeid(std::pair<char32_t, char32_t>)) {
          ranges.push_back(std::any_cast<std::pair<char32_t, char32_t>>(v));
        } else {
          const auto &vec =
              std::any_cast<const std::vector<std::pair<char32_t, char32_t>> &>(
                  v);
          ranges.insert(ranges.end(), vec.begin(), vec.end());
        }
      }
      return ranges;
    };

    g["Class"] = [collect_ranges](const SemanticValues &vs) {
      return cls(collect_ranges(vs));
    };
    g["ClassI"] = [collect_ranges](const SemanticValues &vs) {
      return cls(collect_ranges(vs), true);
    };
    g["NegatedClass"] = [collect_ranges](const SemanticValues &vs) {
      return ncls(collect_ranges(vs));
    };
    g["NegatedClassI"] = [collect_ranges](const SemanticValues &vs) {
      return ncls(collect_ranges(vs), true);
    };
    g["Range"] = [](const SemanticValues &vs) -> std::any {
      switch (vs.choice()) {
      case 0: {
        auto s1 = std::any_cast<std::string>(vs[0]);
        auto s2 = std::any_cast<std::string>(vs[1]);
        auto cp1 = decode_codepoint(s1.data(), s1.length());
        auto cp2 = decode_codepoint(s2.data(), s2.length());
        if (cp1 > cp2) {
          throw SyntaxErrorException("characer range is out of order...",
                                     vs.line_info());
        }
        return std::pair(cp1, cp2);
      }
      case 1: // ClassEscape
      case 2: // PosixClass
        return vs[0];
      case 3: {
        auto s = std::any_cast<std::string>(vs[0]);
        auto cp = decode_codepoint(s.data(), s.length());
        return std::pair(cp, cp);
      }
      }
      return std::pair<char32_t, char32_t>(0, 0);
    };
    g["ClassEscape"] = [](const SemanticValues &vs) {
      auto ch = vs.sv()[1];
      const char *name = nullptr;
      switch (ch) {
      case 'd':
      case 'D': name = "digit"; break;
      case 's':
      case 'S': name = "space"; break;
      default: name = "word"; break;
      }
      auto ranges = *predefined_character_class(name);
      if (ch == 'D' || ch == 'S' || ch == 'W') {
        ranges = complement_character_ranges(ranges);
      }
      return ranges;
    };
    g["PosixClass"] = [](const SemanticValues &vs) {
      auto sv = vs.sv(); // `[:name:]` or `[:^name:]`
      auto negated = sv[2] == '^';
      auto name = sv.substr(negated ? 3 : 2, sv.size() - (negated ? 5 : 4));
      auto ranges = predefined_character_class(name);
      if (!ranges) {
        auto msg = "invalid POSIX character class '" + std::string(name) + "'";
        throw SyntaxErrorException(msg.c_str(), vs.line_info());
      }
      return negated ? complement_character_ranges(*ranges) : *ranges;
    };
    g["Char"] = [](const SemanticValues &vs) {
      return resolve_escape_sequence(vs.sv().data(), vs.sv().length());
    };

    g["RepetitionRange"] = [&](const SemanticValues &vs) {
      switch (vs.choice()) {
      case 0: { // Number COMMA Number
        auto min = std::any_cast<size_t>(vs[0]);
        auto max = std::any_cast<size_t>(vs[1]);
        return std::pair(min, max);
      }
      case 1: // Number COMMA
        return std::pair(std::any_cast<size_t>(vs[0]),
                         std::numeric_limits<size_t>::max());
      case 2: { // Number
        auto n = std::any_cast<size_t>(vs[0]);
        return std::pair(n, n);
      }
      default: // COMMA Number
        return std::pair(std::numeric_limits<size_t>::min(),
                         std::any_cast<size_t>(vs[0]));
      }
    };
    g["Number"] = [&](const SemanticValues &vs) {
      return vs.token_to_number<size_t>();
    };

    g["CapScope"].enter = [](const Context & /*c*/, const char * /*s*/,
                             size_t /*n*/, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);
      data.captures_stack.emplace_back();
    };
    g["CapScope"].leave = [](const Context & /*c*/, const char * /*s*/,
                             size_t /*n*/, size_t /*matchlen*/,
                             std::any & /*value*/, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);
      data.captures_stack.pop_back();
    };

    g["AND"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["NOT"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["QUESTION"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["STAR"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["PLUS"] = [](const SemanticValues &vs) { return *vs.sv().data(); };

    g["DOT"] = [](const SemanticValues & /*vs*/) { return dot(); };

    g["CUT"] = [](const SemanticValues & /*vs*/) { return cut(); };

    g["BeginCap"] = [](const SemanticValues &vs) { return vs.token(); };

    g["BackRef"] = [&](const SemanticValues &vs, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);

      // Undefined back reference check
      {
        auto found = false;
        auto it = data.captures_stack.rbegin();
        while (it != data.captures_stack.rend()) {
          if (it->find(vs.token()) != it->end()) {
            found = true;
            break;
          }
          ++it;
        }
        if (!found) {
          auto ptr = vs.token().data() - 1; // include '$' symbol
          data.undefined_back_references.emplace_back(vs.token(), ptr);
        }
      }

      // NOTE: Disable packrat parsing if a back reference is not defined in
      // captures in the current definition rule.
      if (data.captures_in_current_definition.find(vs.token()) ==
          data.captures_in_current_definition.end()) {
        data.enablePackratParsing = false;
      }

      return bkr(vs.token_to_string());
    };

    g["Ignore"] = [](const SemanticValues &vs) { return vs.size() > 0; };

    g["Parameters"] = [](const SemanticValues &vs) {
      return vs.transform<std::string>();
    };

    g["Arguments"] = [](const SemanticValues &vs) {
      return vs.transform<std::shared_ptr<Ope>>();
    };

    g["PrecedenceClimbing"] = [](const SemanticValues &vs) {
      PrecedenceClimbing::BinOpeInfo binOpeInfo;
      size_t level = 1;
      for (const auto &v : vs) {
        auto tokens = std::any_cast<std::vector<std::string_view>>(v);
        auto assoc = tokens[0][0];
        for (size_t i = 1; i < tokens.size(); i++) {
          binOpeInfo[tokens[i]] = std::pair(level, assoc);
        }
        level++;
      }
      Instruction instruction;
      instruction.type = "precedence";
      instruction.data = binOpeInfo;
      instruction.sv = vs.sv();
      return instruction;
    };
    g["PrecedenceInfo"] = [](const SemanticValues &vs) {
      return vs.transform<std::string_view>();
    };
    g["PrecedenceOpe"] = [](const SemanticValues &vs) { return vs.token(); };
    g["PrecedenceAssoc"] = [](const SemanticValues &vs) { return vs.token(); };

    g["ErrorMessage"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "error_message";
      instruction.data = std::any_cast<std::string>(vs[0]);
      instruction.sv = vs.sv();
      return instruction;
    };

    g["NoAstOpt"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "no_ast_opt";
      instruction.sv = vs.sv();
      return instruction;
    };

    g["NoWhitespace"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "no_whitespace";
      instruction.sv = vs.sv();
      return instruction;
    };

    g["AstName"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "ast_name";
      instruction.data = std::any_cast<std::string>(vs[0]);
      instruction.sv = vs.sv();
      return instruction;
    };

    g["Instruction"] = [](const SemanticValues &vs) {
      return vs.transform<Instruction>();
    };
  }

  bool apply_precedence_instruction(Definition &rule,
                                    const PrecedenceClimbing::BinOpeInfo &info,
                                    const char *s, Log log) {
    try {
      auto &seq = dynamic_cast<Sequence &>(*rule.get_core_operator());
      auto atom = seq.opes_[0];
      auto &rep = dynamic_cast<Repetition &>(*seq.opes_[1]);
      auto &seq1 = dynamic_cast<Sequence &>(*rep.ope_);
      auto binop = seq1.opes_[0];
      auto atom1 = seq1.opes_[1];

      auto atom_name = dynamic_cast<Reference &>(*atom).name_;
      auto binop_name = dynamic_cast<Reference &>(*binop).name_;
      auto atom1_name = dynamic_cast<Reference &>(*atom1).name_;

      if (!rep.is_zom() || atom_name != atom1_name || atom_name == binop_name) {
        if (log) {
          auto line = line_info(s, rule.s_);
          log(line.first, line.second,
              "'precedence' instruction cannot be applied to '" + rule.name +
                  "'.",
              "");
        }
        return false;
      }

      rule.holder_->ope_ = pre(atom, binop, info, rule);
      rule.disable_action = true;
    } catch (...) {
      if (log) {
        auto line = line_info(s, rule.s_);
        log(line.first, line.second,
            "'precedence' instruction cannot be applied to '" + rule.name +
                "'.",
            "");
      }
      return false;
    }
    return true;
  }

  ParserContext perform_core(const char *s, size_t n, const Rules &rules,
                             Log log, std::string requested_start,
                             bool enable_left_recursion = true) {
    Data data;
    auto &grammar = *data.grammar;

    // Built-in macros
    {
      // `%recover`
      {
        auto &rule = grammar[RECOVER_DEFINITION_NAME];
        rule <= ref(grammar, "x", "", false, {});
        rule.name = RECOVER_DEFINITION_NAME;
        rule.s_ = "[native]";
        rule.ignoreSemanticValue = true;
        rule.is_macro = true;
        rule.params = {"x"};
      }
    }

    try {
      std::any dt = &data;
      auto r = g["Grammar"].parse(s, n, dt, nullptr, log);

      if (!r.ret) {
        if (log) {
          if (r.error_info.message_pos) {
            auto line = line_info(s, r.error_info.message_pos);
            log(line.first, line.second, r.error_info.message,
                r.error_info.label);
          } else {
            auto line = line_info(s, r.error_info.error_pos);
            log(line.first, line.second, "syntax error", r.error_info.label);
          }
        }
        return {};
      }
    } catch (const SyntaxErrorException &e) {
      if (log) {
        auto line = e.line_info();
        log(line.first, line.second, e.what(), "");
      }
      return {};
    }

    // User provided rules
    for (auto [user_name, user_rule] : rules) {
      auto name = user_name;
      auto ignore = false;
      if (!name.empty() && name[0] == '~') {
        ignore = true;
        name.erase(0, 1);
      }
      if (!name.empty()) {
        auto &rule = grammar[name];
        rule <= user_rule;
        rule.name = name;
        rule.ignoreSemanticValue = ignore;
      }
    }

    // Check duplicated definitions
    auto ret = true;

    if (!data.duplicates_of_definition.empty()) {
      for (const auto &[name, ptr] : data.duplicates_of_definition) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second,
              "the definition '" + name + "' is already defined.", "");
        }
      }
      ret = false;
    }

    // Check duplicated instructions
    if (!data.duplicates_of_instruction.empty()) {
      for (const auto &[type, ptr] : data.duplicates_of_instruction) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second,
              "the instruction '" + type + "' is already defined.", "");
        }
      }
      ret = false;
    }

    // Check undefined back references
    if (!data.undefined_back_references.empty()) {
      for (const auto &[name, ptr] : data.undefined_back_references) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second,
              "the back reference '" + name + "' is undefined.", "");
        }
      }
      ret = false;
    }

    // Set root definition
    auto start = data.start;

    if (!requested_start.empty()) {
      if (grammar.count(requested_start)) {
        start = requested_start;
      } else {
        if (log) {
          auto line = line_info(s, s);
          log(line.first, line.second,
              "the specified start rule '" + requested_start +
                  "' is undefined.",
              "");
        }
        ret = false;
      }
    }

    if (!ret) { return {}; }

    auto &start_rule = grammar[start];

    // Check if the start rule has ignore operator
    {
      if (start_rule.ignoreSemanticValue) {
        if (log) {
          auto line = line_info(s, start_rule.s_);
          log(line.first, line.second,
              "ignore operator cannot be applied to '" + start_rule.name + "'.",
              "");
        }
        ret = false;
      }
    }

    if (!ret) { return {}; }

    // Check missing definitions
    auto referenced = std::unordered_set<std::string>{
        WHITESPACE_DEFINITION_NAME,
        WORD_DEFINITION_NAME,
        RECOVER_DEFINITION_NAME,
        start_rule.name,
    };

    for (auto &[_, rule] : grammar) {
      ReferenceChecker vis(grammar, rule.params);
      rule.accept(vis);
      referenced.insert(vis.referenced.begin(), vis.referenced.end());
      for (const auto &[name, ptr] : vis.error_s) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second, vis.error_message[name], "");
        }
        ret = false;
      }
    }

    for (auto &[name, rule] : grammar) {
      if (!referenced.count(name)) {
        if (log) {
          auto line = line_info(s, rule.s_);
          auto msg = "'" + name + "' is not referenced.";
          log(line.first, line.second, msg, "");
        }
      }
    }

    if (!ret) { return {}; }

    // Link references
    for (auto &x : grammar) {
      auto &rule = x.second;
      LinkReferences vis(grammar, rule.params);
      rule.accept(vis);
    }

    // Compute can_be_empty for each rule (fixed-point iteration)
    {
      bool changed = true;
      while (changed) {
        changed = false;
        for (auto &[name, rule] : grammar) {
          ComputeCanBeEmpty vis;
          rule.accept(vis);
          if (vis.result != rule.can_be_empty) {
            rule.can_be_empty = vis.result;
            changed = true;
          }
        }
      }
    }

    // Check left recursion
    if (enable_left_recursion) {
      for (auto &[name, rule] : grammar) {
        DetectLeftRecursion vis(name);
        rule.accept(vis);
        if (vis.error_s) { rule.is_left_recursive = true; }
      }
    } else {
      ret = true;

      for (auto &[name, rule] : grammar) {
        DetectLeftRecursion vis(name);
        rule.accept(vis);
        if (vis.error_s) {
          if (log) {
            auto line = line_info(s, vis.error_s);
            log(line.first, line.second, "'" + name + "' is left recursive.",
                "");
          }
          ret = false;
        }
      }

      if (!ret) { return {}; }
    }

    // Check infinite loop
    if (detect_infiniteLoop(data, start_rule, log, s)) { return {}; }

    // Automatic whitespace skipping
    if (grammar.count(WHITESPACE_DEFINITION_NAME)) {
      for (auto &x : grammar) {
        auto &rule = x.second;
        auto ope = rule.get_core_operator();
        if (IsLiteralToken::check(*ope)) { rule <= tok(ope); }
      }

      auto &rule = grammar[WHITESPACE_DEFINITION_NAME];
      start_rule.whitespaceOpe = wsp(rule.get_core_operator());

      if (detect_infiniteLoop(data, rule, log, s)) { return {}; }
    }

    // Word expression
    if (grammar.count(WORD_DEFINITION_NAME)) {
      auto &rule = grammar[WORD_DEFINITION_NAME];
      start_rule.wordOpe = rule.get_core_operator();

      if (detect_infiniteLoop(data, rule, log, s)) { return {}; }
    }

    // Apply instructions
    for (const auto &[name, instructions] : data.instructions) {
      auto &rule = grammar[name];

      for (const auto &instruction : instructions) {
        if (instruction.type == "precedence") {
          const auto &info =
              std::any_cast<PrecedenceClimbing::BinOpeInfo>(instruction.data);

          if (!apply_precedence_instruction(rule, info, s, log)) { return {}; }
        } else if (instruction.type == "error_message") {
          rule.error_message = std::any_cast<std::string>(instruction.data);
        } else if (instruction.type == "no_ast_opt") {
          rule.no_ast_opt = true;
        } else if (instruction.type == "no_whitespace") {
          rule.no_whitespace = true;
        } else if (instruction.type == "ast_name") {
          rule.ast_name = std::any_cast<std::string>(instruction.data);
        }
      }
    }

    // Setup First-Set and ISpan optimizations. A single visitor is shared
    // across all rules so its first-set cache and visited-rule set persist:
    // each rule's first-sets are computed once (O(N)) instead of re-walking
    // every reachable rule once per referencing rule (O(N^2)).
    {
      SetupFirstSets vis;
      for (auto &x : grammar) {
        x.second.accept(vis);
      }
    }

    return {data.grammar, start, data.enablePackratParsing};
  }

  bool detect_infiniteLoop(const Data &data, Definition &rule, const Log &log,
                           const char *s) const {
    std::vector<std::pair<const char *, std::string>> refs;
    std::unordered_map<std::string, bool> has_error_cache;
    DetectInfiniteLoop vis(data.start_pos, rule.name, refs, has_error_cache);
    rule.accept(vis);
    if (vis.has_error) {
      if (log) {
        auto line = line_info(s, vis.error_s);
        log(line.first, line.second,
            "infinite loop is detected in '" + vis.error_name + "'.", "");
      }
      return true;
    }
    return false;
  }

  Grammar g;
};


parser::parser(const char *s, size_t n, const Rules &rules,
               std::string_view start) {
  load_grammar(s, n, rules, start);
}

parser::parser(const char *s, size_t n, std::string_view start)
    : parser(s, n, Rules(), start) {}

parser::parser(std::string_view sv, const Rules &rules,
               std::string_view start)
    : parser(sv.data(), sv.size(), rules, start) {}

parser::parser(std::string_view sv, std::string_view start)
    : parser(sv.data(), sv.size(), Rules(), start) {}

#if defined(__cpp_lib_char8_t)
parser::parser(std::u8string_view sv, const Rules &rules,
               std::string_view start)
    : parser(reinterpret_cast<const char *>(sv.data()), sv.size(), rules,
             start) {}

parser::parser(std::u8string_view sv, std::string_view start)
    : parser(reinterpret_cast<const char *>(sv.data()), sv.size(), Rules(),
             start) {}
#endif

parser::operator bool() const { return grammar_ != nullptr; }

bool parser::load_grammar(const char *s, size_t n, const Rules &rules,
                          std::string_view start) {
  auto cxt = ParserGenerator::parse(s, n, rules, log_, start,
                                    enableLeftRecursion_);
  grammar_ = cxt.grammar;
  start_ = cxt.start;
  enablePackratParsing_ = cxt.enablePackratParsing;
  return grammar_ != nullptr;
}

bool parser::load_grammar(const char *s, size_t n, std::string_view start) {
  return load_grammar(s, n, Rules(), start);
}

bool parser::load_grammar(std::string_view sv, const Rules &rules,
                          std::string_view start) {
  return load_grammar(sv.data(), sv.size(), rules, start);
}

bool parser::load_grammar(std::string_view sv, std::string_view start) {
  return load_grammar(sv.data(), sv.size(), Rules(), start);
}

std::vector<uint8_t> parser::serialize_grammar() const {
  return GrammarBlob::serialize(*grammar_, start_);
}

bool parser::load_blob(const std::vector<uint8_t> &blob) {
  try {
    grammar_ = GrammarBlob::deserialize(blob, start_);
  } catch (const std::exception &) { return false; }
  if (grammar_ != nullptr) {
    enablePackratParsing_ = (*grammar_)[start_].enablePackratParsing;
  }
  return grammar_ != nullptr;
}

bool parser::parse_n(const char *s, size_t n, const char *path) const {
  if (grammar_ != nullptr) {
    const auto &rule = (*grammar_)[start_];
    auto result = rule.parse(s, n, path, log_, error_reporter_);
    return post_process(s, n, result);
  }
  return false;
}

bool parser::parse_n(const char *s, size_t n, std::any &dt,
                     const char *path) const {
  if (grammar_ != nullptr) {
    const auto &rule = (*grammar_)[start_];
    auto result = rule.parse(s, n, dt, path, log_, error_reporter_);
    return post_process(s, n, result);
  }
  return false;
}

bool parser::parse(std::string_view sv, const char *path) const {
  return parse_n(sv.data(), sv.size(), path);
}

bool parser::parse(std::string_view sv, std::any &dt,
                   const char *path) const {
  return parse_n(sv.data(), sv.size(), dt, path);
}

#if defined(__cpp_lib_char8_t)
bool parser::parse(std::u8string_view sv, const char *path) const {
  return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), path);
}

bool parser::parse(std::u8string_view sv, std::any &dt,
                   const char *path) const {
  return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), dt,
                 path);
}
#endif

Definition &parser::operator[](const char *s) { return (*grammar_)[s]; }

const Definition &parser::operator[](const char *s) const {
  return (*grammar_)[s];
}

const Grammar &parser::get_grammar() const { return *grammar_; }

void parser::disable_eoi_check() {
  if (grammar_ != nullptr) {
    auto &rule = (*grammar_)[start_];
    rule.eoi_check = false;
  }
}

void parser::enable_left_recursion(bool enable) { enableLeftRecursion_ = enable; }

void parser::enable_packrat_parsing() {
  if (grammar_ != nullptr) {
    auto &rule = (*grammar_)[start_];
    rule.enablePackratParsing = enablePackratParsing_;
  }
}

void parser::enable_trace(TracerEnter tracer_enter, TracerLeave tracer_leave) {
  if (grammar_ != nullptr) {
    auto &rule = (*grammar_)[start_];
    rule.tracer_enter = tracer_enter;
    rule.tracer_leave = tracer_leave;
  }
}

void parser::enable_trace(TracerEnter tracer_enter, TracerLeave tracer_leave,
                          TracerStartOrEnd tracer_start,
                          TracerStartOrEnd tracer_end) {
  if (grammar_ != nullptr) {
    auto &rule = (*grammar_)[start_];
    rule.tracer_enter = tracer_enter;
    rule.tracer_leave = tracer_leave;
    rule.tracer_start = tracer_start;
    rule.tracer_end = tracer_end;
  }
}

void parser::set_verbose_trace(bool verbose_trace) {
  if (grammar_ != nullptr) {
    auto &rule = (*grammar_)[start_];
    rule.verbose_trace = verbose_trace;
  }
}

void parser::set_logger(Log log) { log_ = log; }

void parser::set_error_reporter(ErrorReporter reporter) {
  error_reporter_ = reporter;
}

void parser::set_logger(
    std::function<void(size_t line, size_t col, const std::string &msg)> log) {
  log_ = [log](size_t line, size_t col, const std::string &msg,
               const std::string & /*rule*/) { log(line, col, msg); };
}

bool parser::post_process(const char *s, size_t n,
                          Definition::Result &r) const {
  if ((log_ || error_reporter_) && !r.ret) {
    r.error_info.output_log(log_, error_reporter_, s, n);
  }
  return r.ret && !r.recovered;
}

std::vector<std::string> parser::get_no_ast_opt_rules() const {
  std::vector<std::string> rules;
  for (auto &[name, rule] : *grammar_) {
    if (rule.no_ast_opt) {
      rules.push_back(rule.ast_name.empty() ? name : rule.ast_name);
    }
  }
  return rules;
}

/*-----------------------------------------------------------------------------
 *  enable_tracing
 *---------------------------------------------------------------------------*/

void enable_tracing(parser &parser, std::ostream &os) {
  parser.enable_trace(
      [&](auto &ope, auto s, auto, auto &, auto &c, auto &, auto &trace_data) {
        auto prev_pos = std::any_cast<size_t>(trace_data);
        auto pos = static_cast<size_t>(s - c.s);
        auto backtrack = (pos < prev_pos ? "*" : "");
        std::string indent;
        auto level = c.trace_ids.size() - 1;
        while (level--) {
          indent += "│";
        }
        std::string name;
        {
          name = peg::TraceOpeName::get(const_cast<peg::Ope &>(ope));

          auto lit = dynamic_cast<const peg::LiteralString *>(&ope);
          if (lit) { name += " '" + peg::escape_characters(lit->lit_) + "'"; }
        }
        os << "E " << pos + 1 << backtrack << "\t" << indent << "┌" << name
           << " #" << c.trace_ids.back() << std::endl;
        trace_data = static_cast<size_t>(pos);
      },
      [&](auto &ope, auto s, auto, auto &sv, auto &c, auto &, auto len,
          auto &) {
        auto pos = static_cast<size_t>(s - c.s);
        if (len != static_cast<size_t>(-1)) { pos += len; }
        std::string indent;
        auto level = c.trace_ids.size() - 1;
        while (level--) {
          indent += "│";
        }
        auto ret = len != static_cast<size_t>(-1) ? "└o " : "└x ";
        auto name = peg::TraceOpeName::get(const_cast<peg::Ope &>(ope));
        std::stringstream choice;
        if (sv.choice_count() > 0) {
          choice << " " << sv.choice() << "/" << sv.choice_count();
        }
        std::string token;
        if (!sv.tokens.empty()) {
          token += ", token '";
          token += sv.tokens[0];
          token += "'";
        }
        std::string matched;
        if (peg::success(len) &&
            peg::TokenChecker::is_token(const_cast<peg::Ope &>(ope))) {
          matched = ", match '" + peg::escape_characters(s, len) + "'";
        }
        os << "L " << pos + 1 << "\t" << indent << ret << name << " #"
           << c.trace_ids.back() << choice.str() << token << matched
           << std::endl;
      },
      [&](auto &trace_data) { trace_data = static_cast<size_t>(0); },
      [&](auto &) {});
}

/*-----------------------------------------------------------------------------
 *  enable_profiling
 *---------------------------------------------------------------------------*/

void enable_profiling(parser &parser, std::ostream &os) {
  struct Stats {
    struct Item {
      std::string name;
      size_t success;
      size_t fail;
    };
    std::vector<Item> items;
    std::map<std::string, size_t> index;
    size_t total = 0;
    std::chrono::steady_clock::time_point start;
  };

  parser.enable_trace(
      [&](auto &ope, auto, auto, auto &, auto &, auto &, std::any &trace_data) {
        if (auto holder = dynamic_cast<const peg::Holder *>(&ope)) {
          auto &stats = *std::any_cast<Stats *>(trace_data);

          auto &name = holder->name();
          if (stats.index.find(name) == stats.index.end()) {
            stats.index[name] = stats.index.size();
            stats.items.push_back({name, 0, 0});
          }
          stats.total++;
        }
      },
      [&](auto &ope, auto, auto, auto &, auto &, auto &, auto len,
          std::any &trace_data) {
        if (auto holder = dynamic_cast<const peg::Holder *>(&ope)) {
          auto &stats = *std::any_cast<Stats *>(trace_data);

          auto &name = holder->name();
          auto index = stats.index[name];
          auto &stat = stats.items[index];
          if (len != static_cast<size_t>(-1)) {
            stat.success++;
          } else {
            stat.fail++;
          }

          if (index == 0) {
            auto end = std::chrono::steady_clock::now();
            auto nano = std::chrono::duration_cast<std::chrono::microseconds>(
                            end - stats.start)
                            .count();
            auto sec = nano / 1000000.0;
            os << "duration: " << sec << "s (" << nano << "µs)" << std::endl
               << std::endl;

            char buff[BUFSIZ];
            size_t total_success = 0;
            size_t total_fail = 0;
            for (auto &[name, success, fail] : stats.items) {
              total_success += success;
              total_fail += fail;
            }

            os << "  id       total      %     success        fail  "
                  "definition"
               << std::endl;

            auto grand_total = total_success + total_fail;
            snprintf(buff, BUFSIZ, "%4s  %10zu  %5s  %10zu  %10zu  %s", "",
                     grand_total, "", total_success, total_fail,
                     "Total counters");
            os << buff << std::endl;

            snprintf(buff, BUFSIZ, "%4s  %10s  %5s  %10.2f  %10.2f  %s", "", "",
                     "", total_success * 100.0 / grand_total,
                     total_fail * 100.0 / grand_total, "% success/fail");
            os << buff << std::endl << std::endl;
            ;

            size_t id = 0;
            for (auto &[name, success, fail] : stats.items) {
              auto total = success + fail;
              auto ratio = total * 100.0 / stats.total;
              snprintf(buff, BUFSIZ, "%4zu  %10zu  %5.2f  %10zu  %10zu  %s", id,
                       total, ratio, success, fail, name.c_str());
              os << buff << std::endl;
              id++;
            }
          }
        }
      },
      [&](auto &trace_data) {
        auto stats = new Stats{};
        stats->start = std::chrono::steady_clock::now();
        trace_data = stats;
      },
      [&](auto &trace_data) {
        auto stats = std::any_cast<Stats *>(trace_data);
        delete stats;
      });
}
} // namespace peg
