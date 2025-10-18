// include/tac.hpp
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "ast.hpp"

namespace TAC {

    struct Emitter {
        int tmp = 0, lbl = 0;
        std::vector<std::string> out;

        std::string newT() { std::ostringstream s; s << "t" << (++tmp); return s.str(); }
        std::string newL() { std::ostringstream s; s << "L" << (++lbl); return s.str(); }
        void emit(const std::string& s) { out.push_back(s); }

        // Згенерувати умовні стрибки: якщо e істинне -> Ltrue, інакше -> Lfalse
        void genCond(const AST::Expr* e, const std::string& Ltrue, const std::string& Lfalse) {
            using namespace AST;

            // NOT: інвертуємо гілки
            if (auto u = dynamic_cast<const Unary*>(e)) {
                if (u->op == UnOp::Not) { genCond(u->E.get(), Lfalse, Ltrue); return; }
            }

            // Логічне І (коротке замикання)
            if (auto b = dynamic_cast<const Binary*>(e)) {
                if (b->op == BinOp::And) {
                    auto Lmid = newL();
                    genCond(b->L.get(), Lmid, Lfalse);
                    emit(Lmid + ":");
                    genCond(b->R.get(), Ltrue, Lfalse);
                    return;
                }
                // Логічне АБО (коротке замикання)
                if (b->op == BinOp::Or) {
                    auto Lmid = newL();
                    genCond(b->L.get(), Ltrue, Lmid);
                    emit(Lmid + ":");
                    genCond(b->R.get(), Ltrue, Lfalse);
                    return;
                }
                // Порівняння: відразу в умовний перехід, без tN
                auto rel = [&](BinOp op)->const char* {
                    switch (op) {
                    case BinOp::LT: return "<";  case BinOp::LE: return "<=";
                    case BinOp::GT: return ">";  case BinOp::GE: return ">=";
                    case BinOp::EQ: return "=="; case BinOp::NE: return "!=";
                    default: return nullptr;
                    }
                    };
                if (auto op = rel(b->op)) {
                    auto a = genExpr(b->L.get());
                    auto c = genExpr(b->R.get());
                    emit(std::string("if ") + a + " " + op + " " + c + " goto " + Ltrue);
                    emit("goto " + Lfalse);
                    return;
                }
            }

            // Загальний випадок: обчислити вираз і порівняти з нулем
            auto v = genExpr(e);
            emit("if " + v + " goto " + Ltrue);
            emit("goto " + Lfalse);
        }

        // ---------- expressions ----------
        std::string genExpr(const AST::Expr* e) {
            using namespace AST;

            if (auto n = dynamic_cast<const Number*>(e)) {
                std::ostringstream s; s << std::setprecision(12) << n->value; return s.str();
            }
            if (auto id = dynamic_cast<const Ident*>(e)) {
                return id->name;
            }
            if (auto u = dynamic_cast<const Unary*>(e)) {
                auto v = genExpr(u->E.get());
                auto t = newT();
                if (u->op == UnOp::Neg) {
                    emit(t + " = - " + v);
                }
                else { // Not
                    auto ltrue = newL(), lend = newL();
                    emit("if " + v + " goto " + ltrue);
                    emit(t + " = 1");
                    emit("goto " + lend);
                    emit(ltrue + ":");
                    emit(t + " = 0");
                    emit(lend + ":");
                }
                return t;
            }
            if (auto b = dynamic_cast<const Binary*>(e)) {
                auto a = genExpr(b->L.get());
                auto c = genExpr(b->R.get());
                auto t = newT();

                switch (b->op) {
                case BinOp::Add: emit(t + " = " + a + " + " + c); break;
                case BinOp::Sub: emit(t + " = " + a + " - " + c); break;
                case BinOp::Mul: emit(t + " = " + a + " * " + c); break;
                case BinOp::Div: emit(t + " = " + a + " / " + c); break;
                case BinOp::Mod: emit(t + " = " + a + " % " + c); break;

                case BinOp::LT: case BinOp::LE: case BinOp::GT:
                case BinOp::GE: case BinOp::EQ: case BinOp::NE: {
                    auto ltrue = newL(), lend = newL();
                    std::string op = (b->op == BinOp::LT ? "<" : b->op == BinOp::LE ? "<=" :
                        b->op == BinOp::GT ? ">" : b->op == BinOp::GE ? ">=" :
                        b->op == BinOp::EQ ? "==" : "!=");
                    emit("if " + a + " " + op + " " + c + " goto " + ltrue);
                    emit(t + " = 0");
                    emit("goto " + lend);
                    emit(ltrue + ":");
                    emit(t + " = 1");
                    emit(lend + ":");
                    break;
                }

                case BinOp::And: {
                    auto lfalse = newL(), lend = newL();
                    emit("ifFalse " + a + " goto " + lfalse);
                    emit("ifFalse " + c + " goto " + lfalse);
                    emit(t + " = 1");
                    emit("goto " + lend);
                    emit(lfalse + ":");
                    emit(t + " = 0");
                    emit(lend + ":");
                    break;
                }

                case BinOp::Or: {
                    auto ltrue = newL(), lend = newL();
                    emit("if " + a + " goto " + ltrue);
                    emit("if " + c + " goto " + ltrue);
                    emit(t + " = 0");
                    emit("goto " + lend);
                    emit(ltrue + ":");
                    emit(t + " = 1");
                    emit(lend + ":");
                    break;
                }
                }
                return t;
            }

            return "/*unsupported-expr*/";
        }

        // ---------- statements ----------
        void genStmt(const AST::Stmt* s) {
            using namespace AST;

            if (auto vd = dynamic_cast<const VarDecl*>(s)) {
                if (vd->init) {
                    auto v = genExpr(vd->init.get());
                    emit(vd->name + " = " + v);
                }
                else {
                    emit(vd->name + " = 0");
                }
                return;
            }

            if (auto as = dynamic_cast<const Assign*>(s)) {
                auto v = genExpr(as->value.get());
                emit(as->name + " = " + v);
                return;
            }

            if (auto pr = dynamic_cast<const Print*>(s)) {
                auto v = genExpr(pr->what.get());
                emit("print " + v);
                return;
            }

            if (auto iff = dynamic_cast<const AST::If*>(s)) {
                auto Lthen = newL();
                if (!iff->elseS) {
                    // if (cond) then;
                    auto Lend = newL();
                    genCond(iff->cond.get(), Lthen, Lend);
                    emit(Lthen + ":");
                    genStmt(iff->thenS.get());
                    emit(Lend + ":");
                }
                else {
                    // if (cond) then; else else;
                    auto Lelse = newL(), Lend = newL();
                    genCond(iff->cond.get(), Lthen, Lelse);
                    emit(Lthen + ":");
                    genStmt(iff->thenS.get());
                    emit("goto " + Lend);
                    emit(Lelse + ":");
                    genStmt(iff->elseS.get());
                    emit(Lend + ":");
                }
                return;
            }

            if (auto wh = dynamic_cast<const AST::While*>(s)) {
                auto Lbegin = newL(), Lbody = newL(), Lend = newL();
                emit(Lbegin + ":");
                genCond(wh->cond.get(), Lbody, Lend);
                emit(Lbody + ":");
                genStmt(wh->body.get());
                emit("goto " + Lbegin);
                emit(Lend + ":");
                return;
            }

            if (auto bl = dynamic_cast<const Block*>(s)) {
                for (auto& it : bl->items) genStmt(it.get());
                return;
            }
        }

        void gen(const AST::Block* root) { genStmt(root); }
        void write(std::ostream& os) const { for (auto& l : out) os << l << "\n"; }
    };

} // namespace TAC
