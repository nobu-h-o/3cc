#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <vector>

enum class SymbolType {
    VARIABLE,
    FUNCTION,
    PARAMETER,
};

struct Symbol {
    std::string name;
    SymbolType type;
    int value;
    int offset;
    int param_count;

    Symbol(const std::string& n, SymbolType t, int v, int o, int pc)
        : name(n), type(t), value(v), offset(o), param_count(pc) {}
};

class SymbolTable {
private:
    std::vector<Symbol> symbols;
    int stack_offset;

public:
    SymbolTable();
    ~SymbolTable() = default;

    int add(const std::string& name);
    int add_function(const std::string& name, int param_count);
    int add_parameter(const std::string& name);
    int lookup(const std::string& name) const;
    void set(const std::string& name, int value);
    int get(const std::string& name) const;
    int get_offset(const std::string& name) const;
    SymbolType get_type(const std::string& name) const;
    int get_param_count(const std::string& name) const;
    int get_stack_offset() const { return stack_offset; }
};

// C-style interface for compatibility with parser
extern "C" {
    SymbolTable* symtab_create(void);
    void symtab_free(SymbolTable *table);
    int symtab_add(SymbolTable *table, const char *name);
    int symtab_add_function(SymbolTable *table, const char *name, int param_count);
    int symtab_add_parameter(SymbolTable *table, const char *name);
    int symtab_lookup(SymbolTable *table, const char *name);
    void symtab_set(SymbolTable *table, const char *name, int value);
    int symtab_get(SymbolTable *table, const char *name);
    int symtab_get_offset(SymbolTable *table, const char *name);
    int symtab_get_param_count(SymbolTable *table, const char *name);
}

#endif /* SYMTAB_H */
