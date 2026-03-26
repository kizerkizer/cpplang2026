#include "binder/name.hpp"

std::string nameKindToString(NameKind nameKind) {
    switch (nameKind) {
        case NameKind::Variable:
            return "Variable";
        case NameKind::Function:
            return "Function";
        case NameKind::Parameter:
            return "Parameter";
        case NameKind::Type:
            return "Type";
        case NameKind::TypeParameter:
            return "TypeParameter";
        case NameKind::Class:
            return "Class";
        case NameKind::ClassMember:
            return "ClassMember";
        case NameKind::Interface:
            return "Interface";
        case NameKind::InterfaceMember:
            return "InterfaceMember";
        case NameKind::ExtensionalMember:
            return "ExtensionalMember";
        case NameKind::NamespaceMember:
            return "NamespaceMember";
        case NameKind::Builtin:
            return "Builtin";
    }
}

const std::string& Name::getNameString() const {
    return this->nameString;
}

const Scope* Name::getScope() const {
    return this->scope;
}

NameKind Name::getKind() const {
    return this->kind;
}

const Node* Name::getNode() const {
    return this->node;
}