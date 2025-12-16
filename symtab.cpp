#include "symtab.h"
#include <algorithm>

SymbolTable::SymbolTable() : stack_offset(0) {
    symbols.reserve(10);
}

int SymbolTable::lookup(const std::string& name) const {
    for (size_t i = 0; i < symbols.size(); i++) {
        if (symbols[i].name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int SymbolTable::add(const std::string& name) {
    if (lookup(name) >= 0) {
        return -1;
    }

    symbols.emplace_back(name, SymbolType::VARIABLE, 0, stack_offset, 0);
    stack_offset += 4;
    return static_cast<int>(symbols.size() - 1);
}

int SymbolTable::add_function(const std::string& name, int param_count) {
    if (lookup(name) >= 0) {
        return -1;
    }

    symbols.emplace_back(name, SymbolType::FUNCTION, 0, 0, param_count);
    return static_cast<int>(symbols.size() - 1);
}

int SymbolTable::add_parameter(const std::string& name) {
    if (lookup(name) >= 0) {
        return -1;
    }

    symbols.emplace_back(name, SymbolType::PARAMETER, 0, stack_offset, 0);
    stack_offset += 4;
    return static_cast<int>(symbols.size() - 1);
}

void SymbolTable::set(const std::string& name, int value) {
    int idx = lookup(name);
    if (idx >= 0) {
        symbols[idx].value = value;
    }
}

int SymbolTable::get(const std::string& name) const {
    int idx = lookup(name);
    if (idx >= 0) {
        return symbols[idx].value;
    }
    return 0;
}

int SymbolTable::get_offset(const std::string& name) const {
    int idx = lookup(name);
    if (idx >= 0) {
        return symbols[idx].offset;
    }
    return 0;
}

SymbolType SymbolTable::get_type(const std::string& name) const {
    int idx = lookup(name);
    if (idx >= 0) {
        return symbols[idx].type;
    }
    return SymbolType::VARIABLE;
}

int SymbolTable::get_param_count(const std::string& name) const {
    int idx = lookup(name);
    if (idx >= 0) {
        return symbols[idx].param_count;
    }
    return 0;
}

// C-style interface for compatibility with parser
extern "C" {
    SymbolTable* symtab_create(void) {
        return new SymbolTable();
    }

    void symtab_free(SymbolTable *table) {
        delete table;
    }

    int symtab_add(SymbolTable *table, const char *name) {
        return table->add(name);
    }

    int symtab_add_function(SymbolTable *table, const char *name, int param_count) {
        return table->add_function(name, param_count);
    }

    int symtab_add_parameter(SymbolTable *table, const char *name) {
        return table->add_parameter(name);
    }

    int symtab_lookup(SymbolTable *table, const char *name) {
        return table->lookup(name);
    }

    void symtab_set(SymbolTable *table, const char *name, int value) {
        table->set(name, value);
    }

    int symtab_get(SymbolTable *table, const char *name) {
        return table->get(name);
    }

    int symtab_get_offset(SymbolTable *table, const char *name) {
        return table->get_offset(name);
    }

    int symtab_get_param_count(SymbolTable *table, const char *name) {
        return table->get_param_count(name);
    }
}
