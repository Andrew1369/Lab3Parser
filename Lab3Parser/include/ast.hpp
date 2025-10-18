// include/ast.hpp
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <cmath> 
#include <utility>

namespace AST {

    struct Context {
        std::unordered_map<std::string, double> vars;
    };

    struct Node {
        virtual ~Node() = default;
        virtual void emitDOT(std::ostream& out, int& id, int parent = -1) const = 0;
    };

    struct Expr : Node {
        virtual double eval(Context& ctx) const = 0;
    };

    struct Number : Expr {
        double value;
        explicit Number(double v) : value(v) {}
        double eval(Context&) const override { return value; }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"Number(" << std::setprecision(12) << value << ")\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
        }
    };

    struct Ident : Expr {
        std::string name;
        explicit Ident(std::string n) : name(std::move(n)) {}
        double eval(Context& ctx) const override {
            auto it = ctx.vars.find(name);
            if (it == ctx.vars.end()) throw std::runtime_error("undefined variable: " + name);
            return it->second;
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"Ident(" << name << ")\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
        }
    };

    enum class BinOp {
        Add, Sub, Mul, Div, Mod,
        LT, LE, GT, GE, EQ, NE,
        And, Or
    };

    struct Binary : Expr {
        BinOp op;
        std::unique_ptr<Expr> L, R;

        // Ќќ¬≈: в≥д сирих вказ≥вник≥в
        Binary(BinOp o, Expr* l, Expr* r) : op(o), L(l), R(r) {}

        // як було (залишаЇмо):
        Binary(BinOp o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
            : op(o), L(std::move(l)), R(std::move(r)) {
        }
        static double toBool(double x) { return x != 0.0 ? 1.0 : 0.0; }
        double eval(Context& ctx) const override {
            if (op == BinOp::And) {
                return toBool(L->eval(ctx)) && toBool(R->eval(ctx)) ? 1.0 : 0.0;
            }
            if (op == BinOp::Or) {
                return toBool(L->eval(ctx)) || toBool(R->eval(ctx)) ? 1.0 : 0.0;
            }
            double a = L->eval(ctx), b = R->eval(ctx);
            switch (op) {
            case BinOp::Add: return a + b;
            case BinOp::Sub: return a - b;
            case BinOp::Mul: return a * b;
            case BinOp::Div: return b == 0.0 ? throw std::runtime_error("division by zero") : a / b;
            case BinOp::Mod: return std::fmod(a, b);
            case BinOp::LT:  return a < b ? 1.0 : 0.0;
            case BinOp::LE:  return a <= b ? 1.0 : 0.0;
            case BinOp::GT:  return a > b ? 1.0 : 0.0;
            case BinOp::GE:  return a >= b ? 1.0 : 0.0;
            case BinOp::EQ:  return a == b ? 1.0 : 0.0;
            case BinOp::NE:  return a != b ? 1.0 : 0.0;
            default:         return 0.0;
            }
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            auto label = [this]() {
                switch (op) {
                case BinOp::Add: return "+";
                case BinOp::Sub: return "-";
                case BinOp::Mul: return "*";
                case BinOp::Div: return "/";
                case BinOp::Mod: return "%";
                case BinOp::LT:  return "<";
                case BinOp::LE:  return "<=";
                case BinOp::GT:  return ">";
                case BinOp::GE:  return ">=";
                case BinOp::EQ:  return "==";
                case BinOp::NE:  return "!=";
                case BinOp::And: return "&&";
                case BinOp::Or:  return "||";
                }
                return "?";
                }();
            int me = id++;
            out << "  n" << me << " [label=\"Binary(" << label << ")\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            L->emitDOT(out, id, me);
            R->emitDOT(out, id, me);
        }
    };

    enum class UnOp { Neg, Not };

    struct Unary : Expr {
        UnOp op;
        std::unique_ptr<Expr> E;

        // Ќќ¬≈: в≥д сирого вказ≥вника
        Unary(UnOp o, Expr* e) : op(o), E(e) {}

        // як було (залишаЇмо):
        Unary(UnOp o, std::unique_ptr<Expr> e) : op(o), E(std::move(e)) {}
        double eval(Context& ctx) const override {
            double v = E->eval(ctx);
            return op == UnOp::Neg ? -v : (v == 0.0 ? 1.0 : 0.0);
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            const char* label = (op == UnOp::Neg) ? "unary -" : "!";
            int me = id++;
            out << "  n" << me << " [label=\"Unary(" << label << ")\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            E->emitDOT(out, id, me);
        }
    };

    struct Stmt : Node {
        virtual void exec(Context& ctx) const = 0;
    };

    struct Block : Stmt {
        std::vector<std::unique_ptr<Stmt>> items;
        void add(Stmt* s) { items.emplace_back(s); }
        void exec(Context& ctx) const override {
            for (auto& s : items) s->exec(ctx);
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"Block\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            for (auto& s : items) s->emitDOT(out, id, me);
        }
    };

    enum class Type { Int, Double };

    struct VarDecl : Stmt {
        Type type;
        std::string name;
        std::unique_ptr<Expr> init; // may be null
        VarDecl(Type t, std::string n, Expr* e = nullptr) : type(t), name(std::move(n)), init(e) {}
        void exec(Context& ctx) const override {
            double v = 0.0;
            if (init) v = init->eval(ctx);
            ctx.vars[name] = v; // одне сховище double Ч просто й наочно
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"VarDecl(" << (type == Type::Int ? "int" : "double") << " " << name << ")\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            if (init) init->emitDOT(out, id, me);
        }
    };

    struct Assign : Stmt {
        std::string name;
        std::unique_ptr<Expr> value;
        Assign(std::string n, Expr* v) : name(std::move(n)), value(v) {}
        void exec(Context& ctx) const override {
            double v = value->eval(ctx);
            ctx.vars[name] = v;
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"Assign(" << name << ")\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            value->emitDOT(out, id, me);
        }
    };

    struct Print : Stmt {
        std::unique_ptr<Expr> what;
        explicit Print(Expr* e) : what(e) {}
        void exec(Context& ctx) const override {
            double v = what->eval(ctx);
            // друк €к у звичайних мовах: без зайвих нул≥в, але стаб≥льно
            std::cout << std::setprecision(12) << v << std::endl;
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"Print\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            what->emitDOT(out, id, me);
        }
    };

    struct If : Stmt {
        std::unique_ptr<Expr> cond;
        std::unique_ptr<Stmt> thenS;
        std::unique_ptr<Stmt> elseS; // may be null
        If(Expr* c, Stmt* t, Stmt* e = nullptr) : cond(c), thenS(t), elseS(e) {}
        void exec(Context& ctx) const override {
            if (cond->eval(ctx) != 0.0) thenS->exec(ctx);
            else if (elseS) elseS->exec(ctx);
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"If\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            cond->emitDOT(out, id, me);
            thenS->emitDOT(out, id, me);
            if (elseS) elseS->emitDOT(out, id, me);
        }
    };

    struct While : Stmt {
        std::unique_ptr<Expr> cond;
        std::unique_ptr<Stmt> body;
        While(Expr* c, Stmt* b) : cond(c), body(b) {}
        void exec(Context& ctx) const override {
            while (cond->eval(ctx) != 0.0) body->exec(ctx);
        }
        void emitDOT(std::ostream& out, int& id, int parent) const override {
            int me = id++;
            out << "  n" << me << " [label=\"While\"];\n";
            if (parent >= 0) out << "  n" << parent << " -> n" << me << ";\n";
            cond->emitDOT(out, id, me);
            body->emitDOT(out, id, me);
        }
    };

    // ”тил≥та дл€ виводу AST в .dot
    inline void writeDOT(const Node& root, std::ostream& out) {
        out << "digraph AST {\n";
        int id = 0;
        root.emitDOT(out, id, -1);
        out << "}\n";
    }

} // namespace AST
