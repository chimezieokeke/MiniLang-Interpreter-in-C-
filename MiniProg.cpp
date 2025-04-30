#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>

// Forward declarations
struct Value;
struct Subprogram;

// Type aliases for parameters and closures
using Paramlist = std::vector<std::pair<std::string, Value>>;
using ClosureEnv = std::unordered_map<std::string, Value>;
using SubprogramPtr = std::shared_ptr<Subprogram>;

// Value struct
struct Value {
    enum class Type { INT, SUBPROGRAM, CLOSURE } type;
    int int_val;
    SubprogramPtr subprogram;
    ClosureEnv closure_env;

    Value(int val = 0) : type(Type::INT), int_val(val) {}  // Added default constructor
    Value(SubprogramPtr sub) : type(Type::SUBPROGRAM), subprogram(sub) {}
    Value(SubprogramPtr sub, ClosureEnv env) : type(Type::CLOSURE), subprogram(sub), closure_env(env) {}
};

// Subprogram struct
struct Subprogram {
    std::vector<std::string> params;
    std::string body;
    bool is_generic = false;

    Subprogram(std::vector<std::string> p, std::string b) : params(p), body(b) {}
};

// Environment class
class Environment {
public:
    Environment(bool dynamic_scoping = false) : dynamic_scoping(dynamic_scoping) {
        scopes.emplace_back();
    }
    
    void enter_scope() { scopes.emplace_back(); }
    
    void exit_scope() { 
        if (scopes.size() > 1) scopes.pop_back();
    }
    
    void define(const std::string& name, Value value) {
        scopes.back()[name] = value;
    }

    Value lookup(const std::string& name) {
        if (dynamic_scoping) {
            for (int i = scopes.size() - 1; i >= 0; --i) {
                if (scopes[i].find(name) != scopes[i].end()) {
                    return scopes[i][name];
                }
            }
        } else {
            if (scopes.back().find(name) != scopes.back().end()) {
                return scopes.back()[name];
            }
        }
        throw std::runtime_error("Variable not found: " + name);
    }

private:
    std::vector<ClosureEnv> scopes;
    bool dynamic_scoping;
};

// MiniLang Interpreter class
class MiniLangInterpreter {
public:
    MiniLangInterpreter() : env(false) {}
    
    void define_subprogram(const std::string& name, std::vector<std::string> params, std::string body) {
        auto sub = std::make_shared<Subprogram>(params, body);
        env.define(name, Value(sub));
    }
    
    Value call_subprogram(const std::string& name, Paramlist args, bool indirect = false) {
        Value sub_val = env.lookup(name);
        SubprogramPtr sub = (indirect && sub_val.type == Value::Type::CLOSURE) ? 
                           sub_val.subprogram : sub_val.subprogram;
        
        env.enter_scope();
        for (const auto& arg : args) {
            env.define(arg.first, arg.second);
        }
        Value result = execute_body(sub->body);
        env.exit_scope();
        return result;
    }
    
private:
    Value execute_body(const std::string& body) {
        if (body.find("return") == 0) {
            std::string expr = body.substr(7); // Skip "return "
            if (expr.find("+") != std::string::npos) {
                size_t op_pos = expr.find("+");
                std::string var1 = expr.substr(0, op_pos);
                std::string var2 = expr.substr(op_pos + 1);
                // Trim whitespace
                var1.erase(var1.find_last_not_of(" \t") + 1);
                var2.erase(0, var2.find_first_not_of(" \t"));
                auto val1 = env.lookup(var1);
                auto val2 = env.lookup(var2);
                return Value(val1.int_val + val2.int_val);
            }
        }
        return Value(std::stoi(body));
    }
    
    Environment env;
};

int main() {
    MiniLangInterpreter interpreter;
    
    // Test simple subprogram
    interpreter.define_subprogram("add", {"a", "b"}, "return a + b");
    Paramlist args = {{"a", Value(3)}, {"b", Value(5)}};
    Value result = interpreter.call_subprogram("add", args);
    std::cout << "Simple Subprogram Result: " << result.int_val << std::endl;
    
    return 0;
}