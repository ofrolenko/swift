//===--- Requirement.h - Swift Requirement ASTs -----------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file defines the Requirement class and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_AST_REQUIREMENT_H
#define SWIFT_AST_REQUIREMENT_H

#include "swift/AST/LayoutConstraint.h"
#include "swift/AST/RequirementKind.h"
#include "swift/AST/Type.h"
#include "swift/Basic/Debug.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/Support/ErrorHandling.h"

namespace swift {

/// A single requirement placed on the type parameters (or associated
/// types thereof) of a
class Requirement {
  llvm::PointerIntPair<Type, 3, RequirementKind> FirstTypeAndKind;
  /// The second element of the requirement. Its content is dependent
  /// on the requirement kind.
  /// The payload of the following enum should always match the kind!
  /// Any access to the fields of this enum should first check if the
  /// requested access matches the kind of the requirement.
  union {
    Type SecondType;
    LayoutConstraint SecondLayout;
  };

public:
  /// Create a conformance or same-type requirement.
  Requirement(RequirementKind kind, Type first, Type second)
      : FirstTypeAndKind(first, kind), SecondType(second) {
    assert(first);
    assert(second);
    assert(kind != RequirementKind::Layout);
  }

  /// Create a layout constraint requirement.
  Requirement(RequirementKind kind, Type first, LayoutConstraint second)
      : FirstTypeAndKind(first, kind), SecondLayout(second) {
    assert(first);
    assert(second);
    assert(kind == RequirementKind::Layout);
  }


  /// Determine the kind of requirement.
  RequirementKind getKind() const { return FirstTypeAndKind.getInt(); }

  /// Retrieve the first type.
  Type getFirstType() const {
    return FirstTypeAndKind.getPointer();
  }

  /// Retrieve the second type.
  Type getSecondType() const {
    assert(getKind() != RequirementKind::Layout);
    return SecondType;
  }

  /// Retrieve the layout constraint.
  LayoutConstraint getLayoutConstraint() const {
    assert(getKind() == RequirementKind::Layout);
    return SecondLayout;
  }

  friend llvm::hash_code hash_value(const Requirement &requirement) {
    using llvm::hash_value;

    llvm::hash_code first =
        hash_value(requirement.FirstTypeAndKind.getOpaqueValue());
    llvm::hash_code second;
    switch (requirement.getKind()) {
    case RequirementKind::SameShape:
    case RequirementKind::Conformance:
    case RequirementKind::Superclass:
    case RequirementKind::SameType:
      second = hash_value(requirement.getSecondType());
      break;

    case RequirementKind::Layout:
      second = hash_value(requirement.getLayoutConstraint());
      break;
    }

    return llvm::hash_combine(first, second);
  }

  friend bool operator==(const Requirement &lhs,
                         const Requirement &rhs) {
    if (lhs.FirstTypeAndKind.getOpaqueValue()
          != rhs.FirstTypeAndKind.getOpaqueValue())
      return false;

    switch (lhs.getKind()) {
    case RequirementKind::SameShape:
    case RequirementKind::Conformance:
    case RequirementKind::Superclass:
    case RequirementKind::SameType:
      return lhs.getSecondType().getPointer() ==
          rhs.getSecondType().getPointer();

    case RequirementKind::Layout:
      return lhs.getLayoutConstraint() == rhs.getLayoutConstraint();
    }
    llvm_unreachable("Unhandled RequirementKind in switch");
  }

  /// Whether this requirement's types contain ErrorTypes.
  bool hasError() const;

  /// Whether this requirement is written with canonical types.
  bool isCanonical() const;

  /// Canonicalize the types in this requirement.
  Requirement getCanonical() const;

  /// Subst the types involved in this requirement.
  ///
  /// The \c args arguments are passed through to Type::subst.
  template <typename ...Args>
  Requirement subst(Args &&...args) const {
    auto newFirst = getFirstType().subst(std::forward<Args>(args)...);
    switch (getKind()) {
    case RequirementKind::SameShape:
    case RequirementKind::Conformance:
    case RequirementKind::Superclass:
    case RequirementKind::SameType: {
      auto newSecond = getSecondType().subst(std::forward<Args>(args)...);
      return Requirement(getKind(), newFirst, newSecond);
    }
    case RequirementKind::Layout:
      return Requirement(getKind(), newFirst, getLayoutConstraint());
    }

    llvm_unreachable("Unhandled RequirementKind in switch.");
  }

  ProtocolDecl *getProtocolDecl() const;

  /// Determines if this substituted requirement is satisfied.
  ///
  /// \param conditionalRequirements An out parameter initialized to an
  /// array of requirements that the caller must check to ensure this
  /// requirement is completely satisfied.
  bool isSatisfied(ArrayRef<Requirement> &conditionalRequirements,
                   bool allowMissing = false) const;

  /// Determines if this substituted requirement can ever be satisfied,
  /// possibly with additional substitutions.
  ///
  /// For example, if 'T' is unconstrained, then a superclass requirement
  /// 'T : C' can be satisfied; however, if 'T' already has an unrelated
  /// superclass requirement, 'T : C' cannot be satisfied.
  bool canBeSatisfied() const;

  /// Linear order on requirements in a generic signature.
  int compare(const Requirement &other) const;

  SWIFT_DEBUG_DUMP;
  void dump(raw_ostream &out) const;
  void print(raw_ostream &os, const PrintOptions &opts) const;
  void print(ASTPrinter &printer, const PrintOptions &opts) const;
};

inline void simple_display(llvm::raw_ostream &out, const Requirement &req) {
  req.print(out, PrintOptions());
}

/// A requirement as written in source, together with a source location. See
/// ProtocolDecl::getStructuralRequirements().
struct StructuralRequirement {
  /// The actual requirement, where the types were resolved with the
  /// 'Structural' type resolution stage.
  Requirement req;

  /// The source location where the requirement is written, used for redundancy
  /// and conflict diagnostics.
  SourceLoc loc;

  /// A flag indicating whether the requirement was inferred from the
  /// application of a type constructor. Also used for diagnostics, because
  /// an inferred requirement made redundant by an explicit requirement is not
  /// diagnosed as redundant, since we want to give users the option of
  /// spelling out these requirements explicitly.
  bool inferred = false;
};

} // end namespace swift

#endif
