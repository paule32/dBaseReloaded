#include <cctype>
using namespace std;

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dbasevisitor.h"
#include "dbaseaddnumber.h"
#include "dbaseassign.h"
#include "dbasedownvisitor.h"

#include <QDebug>

#include <QString>
#include <QFile>
#include <QStack>
#include <QByteArray>
#include <QMessageBox>
#include <QTranslator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------
// class for error's in the source.
// error's make a trap - how wonderful is "try catch" :-)
// ------------------------------------------------------
class dBaseError {
public:
    dBaseError(QString message) : m_message(message) {}
    QString m_message;
};

// ------------------------------------------------------
// "catch" newline feeds, and try to handle next line ...
// ------------------------------------------------------
class dBaseNewLine {public:
      dBaseNewLine() {}  };

class dBaseEndOfProgram {public:
      dBaseEndOfProgram() {}  };

class dBaseEndOfComment {public:
      dBaseEndOfComment() {}  };

enum op_enum {
    START_OF_PROGRAM = 0,
    NOP = 1,
    NUMBER_ASSIGN,
    ADD_VAR,
    ADD_NUMBER = 100,
    SUB_NUMBER = 101,
    MUL_NUMBER = 102,
    DIV_NUMBER = 103,
    
    ADD_ASSIGN_VAR_NUMBER = 200,
    
    TYPE_NUMBER = 1000
};

enum enumMathCommand {
    ncMUL
};
enum enumClassType {
    ctAssign,
    ctAssignADD,
    ctAssignSUB,
    ctAssignMUL1,
    ctAssignMUL2,
    ctAssignDIV
};


enum enumStaticType {
    CHAR,
    SHORT,
    INT,
    LONG,
    stFLOAT,
    DOUBLE,
    ARRAY
};

void dBaseAssign   ::accept(dBaseVisitor &v) { v.visit(this); }
void dBaseADDnumber::accept(dBaseVisitor &v) { v.visit(this); }

void testAST()
{
    QVector<dBaseComando *> cmds;
    
    dBaseComando * cmd1 = new dBaseAssign   ; cmds.append(cmd1);
    dBaseComando * cmd2 = new dBaseADDnumber; cmds.append(cmd2);
    
    dBaseDownVisitor down;
    for (int i = 0 ; i < cmds.count(); i++) {
        cmds.at(i)->accept(down);
    }
}


QByteArray buffer, ident_before;

QString number_ident;
QFile * srcfile;
int     char_pos= 0;
int     line_no = 0;

QString ident;

const int MAX_READ_LINE   = 4096;     // maximum of read-in per line
const int TOKEN_IDENT     = 1000;
const int TOKEN_NUMBER    = 1001;
const int TOKEN_DIVASSIGN = 1002;

int in_c_comment = 0;



QMap<QString,QVariant> MyParameters;
QMap<QString,bool>     MyVariables;

// forward declarations ...
int handle_comment();
int handle_space(int c);
int handle_dbaseIcomment(int c);
int handle_Ccomment();
int handle_newline();
int handle_assign();
QVariant handle_ident(int c);

static void inline check_buffer()
{
    if (char_pos >= buffer.size())
    throw new dBaseError(
    QString("buffer overflow."));
}

static int get_char()
{
    char c[] = "";
    if (srcfile->read(c,1) <= 0)
    throw new dBaseEndOfProgram;
    return c[0];
}

static int unget_char()
{
    if (srcfile->pos() > 0)
        srcfile->seek(
        srcfile->pos()-1);
    else
    throw new dBaseError("data position error");
    return get_char();
}

QString handle_number(int c)
{
    QString number;
    number.append(c);
    while (1) {
        c = get_char();
        if ((c >= '0') && (c <= '9')) {
            number.append(c);
            while (1) {
                c = get_char();
                if (c == ' ' || c == '\t' || c == '\n') {
                    if (c == '\n')
                    ++line_no;
                    while (1) {
                        c = get_char();
                        if (c == ' ' || c == '\t' || c == '\n') {
                            if (c == '\n')
                            ++line_no;
                            continue;
                        }   else
                        if (c == '*') { qDebug() << "malser1: " << number;
                            while (1) {
                                c = get_char();
                                if (c >= '0' && c <= '9') {
                                    QString n1 = handle_number(c);
                                    qDebug() << n1;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }   else
                if (c >= '0' && c <= '9') {
                    number.append(c);
                    continue;
                }   else
                if (c == '*') {
                    number.clear();
                    goto malser;
                }   else {
                    break;
                }
            }
        }   else
        if (c == '&') {
            c = get_char();
            if (c == '&')
            handle_newline();
            else return QString("&");
        }   else
        if (c == '/') {
            c = get_char();
            if (c == '*') {
                handle_Ccomment();
                return number;
            }
            else if (c == '/') {
                handle_newline();
                return number;
            }
            return QString("/");
        }   else
        if (c == 0) {
            throw new dBaseEndOfProgram;
        }   else
        if (c == ' ' || c == '\t' || c == '\n') {
            if (c == '\n')
            ++line_no;
            
            while (1) {
                c = get_char();
                if (c == ' ' || c == '\t' || c == '\n') {
                    if (c == '\n')
                    ++line_no;
                    continue;
                }   else
                if (c == '*') { goto malser; }
            }
        }            
        else if (c == '*') { malser:
            qDebug() << "malser2: " << number;
            while (1) {
                c = get_char();
                if (c == ' ' || c == '\t' || c == '\n') {
                    if (c == '\n')
                    ++line_no;
                    continue;
                }   else
                if ((c >= 'a' && c <= 'z') || c == '_') {
                    ident = handle_ident(c).toString();
                    qDebug() << "alpern: " << ident;
                    break;
                }   else
                if (c >= '0' && c <= '9') {
                    //expr_class =
                    //expr_class->addRPNnumber(
                    //expr_class, MUL_NUMBER, number);
                    
                    number.clear(); // <--
                    number.append(c);
                    
                    qDebug() << "male: " << c << number;
                    while (1) {
                        c = get_char();
                        if (c >= '0' && c <= '9') {
                            number.append(c);
                            continue;
                        }   else
                        if ((c >= 'a' && c <= 'z') || c == '_') {
                        //expr_class =
                        //expr_class->addRPNnumber(
                        //expr_class, MUL_NUMBER, number);
                        
                        //expr_class =
                        //expr_class->getCalcResult(
                        //expr_class, ident);
                        
                        
                            //qDebug() << getCalcResult(ident);
                            
                            ident.clear();
                            ident.append(c);
                            handle_ident(c).toString();
                            qDebug() << "id: " << ident;
                            
                            while (1) {
                                c = get_char();
                                if (c == ' ' || c == '\t' || c == '\n') {
                                    if (c == '\n')
                                    ++line_no;
                                    continue;
                                }   else
                                if (c == '=') {
                                    while (1) {
                                        c = get_char();
                                        if (c == ' ' || c == '\t' || c == '\n') {
                                            if (c == '\n')
                                            ++line_no;
                                            continue;
                                        }   else {
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                            
                            number.clear();
                            while (1) {
                                if (c >= '0' && c <= '9') {
                                    number.append(c);
                                    c = get_char();
                                    continue;
                                }
                                else {
                                    qDebug() << "pret: " << number << c;
                                    break;
                                }
                            }
                            break;
                            //goto malser;
                        }   else
                        if (c == ' ' || c == '\t' || c == '\n') {
                            if (c == '\n')
                            ++line_no;
                        }   else
                        if (c == '*') {
                            qDebug() << "malser dreier: " << number;
                            //expr_class =
                            //expr_class->addRPNnumber(
                            //expr_class, MUL_NUMBER, "*-*");
                            
                            //addStackNumber(number);
                            //addStackOp(MUL_NUMBER);
                            
                            goto malser;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    return number;
}

#if 0
int handle_add_variable(int type, QString name, QVariant valu)
{
    MyCommand * my_var = new MyCommand;
    my_var->op    = ADD_VAR;
    my_var->type  = type;
    my_var->name  = name;
    my_var->value = valu;
    exec_cmds.append(my_var);
    
    return 0;
}
#endif

QVariant handle_ident(int c)
{
    QString vname;
    while (1) {
        c = get_char();
        if (((c >= '0') && (c <= '9'))
        ||  ((c >= 'a') && (c <= 'z'))
        ||   (c == '_')) {
            ident.append(c);
            continue;
        }   else
        if (c == '\n' || c == ' '
        ||  c == '\t' || c == '\f' || c == 0) {
            if (c == '\n') ++line_no; else
            if (c == 0) return 0;
            break;
        }   else
        if (c == '*') {
            c = get_char();
            if (c == '*') {
                handle_dbaseIcomment(c);
                break;
            }
            else throw new dBaseError("mul not implemented.");
        }   else
        if (c == '&') {
            c = get_char();
            if (c == '&') {
                handle_dbaseIcomment(c);
                break;
            }
            else throw new dBaseError("and not implemented.");
        }   else
        if (c == '/') {
            c = get_char();
            if (c == '/') {
                handle_newline();
                break;
            }   else
            if (c == '*') {
                handle_Ccomment();
                break;
            }
        }   else
        
        // assign
        if (c == '=') {
            vname = ident;
            qDebug() << "--> " << vname;
            c = get_char();
            
            // ** comment
            if (c == '*') {
                c = get_char();
                if (c == '*') {
                    handle_dbaseIcomment(c);
                    break;
                }
                else throw new dBaseError("mul not implemented.");
            }   else
            
            // && comment
            if (c == '&') {
                c = get_char();
                if (c == '&') {
                    handle_dbaseIcomment(c);
                    break;
                }
                else throw new dBaseError("and not implemented.");
            }   else
            
            // C++ comment
            if (c == '/') {
                c = get_char();
                if (c == '/') {
                    handle_newline();
                    break;
                }   else
                if (c == '*') {
                    handle_Ccomment();
                    break;
                }
            }   else
            if ((c >= '0') && (c <= '9')) {
                qDebug() << "nup: " << handle_number(c);
                return TOKEN_NUMBER;
            }   else
            if (c == '\n' || c == ' '
            ||  c == '\t' || c == '\f') {
                if (c == '\n') ++line_no;
                continue;
            }   else
            if (c == 0)
            throw new dBaseEndOfProgram;  else
            throw new dBaseError("and not implemented.");
        }
    }
    return ident;
}

int handle_newline()
{
    int c = 0;
    while (1) {
        c = get_char();
        if (c == '\n') {
            ++line_no;
            break;
        }
        else if (c == 0)
        return 0;
    }   return 0;
}

int handle_space(int c)
{
    if (c == '\n' || c == ' '
    ||  c == '\t' || c == '\f') {
        if (c == '\n') ++line_no;
        return 1;
    }   return 0;
}

int handle_dbaseIcomment(int c)
{
    if (c == '&' || c == '*')
    {   while(1)
        {   c = get_char();
            if (c == '\n') {
                ++line_no;
                break;
            }   else
            if (c == 0)
            return 0;
        }
    }   else
    return handle_space(c);
    return 0;
}

int handle_Ccomment()
{
    int c = 0;
    while (1) {
        c = get_char();
        if (c == '*') {
            c = get_char();
            if (c == '/')
            return 1;
        }
        else if (c == '\n') {
            ++line_no;
            continue;
        }
        else if (c == 0)
        return 0;
    }   return 1;
}

int handle_assign()
{
    int c;
    while (1) {
        c = handle_comment();
        if (c == '-' || c == '+') {
            if (number_ident.length() <= 1) {
                number_ident.append(c);
                continue;
            }
        }   else
        if (c == TOKEN_NUMBER) { return TOKEN_NUMBER; } else
        if (c == TOKEN_IDENT ) { return TOKEN_IDENT;  } else
        
        if (c == '\n' || c == ' '
        ||  c == '\t' || c == '\f') {
            if (c == '\n') ++line_no;
            continue;
        }   else
        if (c == 0)
        throw new dBaseEndOfProgram;
    }
    return 1;
}

int handle_comment()
{
    int c = 0;
    while (1) {
        c = get_char();
        
        // ** comment
        if (c == '*') {
            c = get_char();
            if (c == '*') {
                handle_newline();
                continue;
            }
        }

        // && comment
        else if (c == '&') {
            c = get_char();
            if (c == '&') {
                handle_newline();
                continue;
            }
        }
        
        // C++ comment
        if (c == '/') {
            c = get_char();
            if (c == '/') {
                handle_newline();
                continue;
            }
            
            // C comment
            else if (c == '*') {
                handle_Ccomment();
                continue;
            }
            
            // division number
            else if ((c >= '0') && (c <= '9')) {
                handle_number(c);
                return TOKEN_NUMBER;
            }
            else if (c == '=')
            return TOKEN_DIVASSIGN; else
            throw new dBaseError("unknow char found.");
        }
        
        // numbers ...
        else if (c == '-' || c == '+') {
            if (number_ident.length() <= 1) {
                number_ident.append(c);
                c = get_char();
                if ((c >= '0') && (c <= '9')) {
                    handle_number(c);
                }  else
                if (c == '\n' || c == ' '
                    ||  c == '&'  || c == '/'  || c == '*'
                    ||  c == '\t' || c == '\f' || c == 0) {
                    if (c == '\n') ++line_no; else
                    if (c == 0) return 0;
                    continue;
                }   else
                    
                // dBase comment I + II
                if (c == '&') handle_dbaseIcomment(c); else
                if (c == '*') handle_dbaseIcomment(c); else 
                    
                // C++ comment
                if (c == '/') {
                    c = get_char();
                    if (c == '/')
                    handle_newline();

                    else if (c == '*') {
                        handle_Ccomment();
                    }
                }   else
                throw new
                dBaseError(
                QString("number format invalid."));
            }
            else
            throw new dBaseError("invalid number format.");
        }
        else if ((c >= '0') && (c <= '9')) {
            QVariant var = handle_number(c).toFloat();
            qDebug() << "num: " << var;
            return TOKEN_NUMBER;
        }
        else if (((c >= 'a') && (c <= 'z')) || (c == '_')) {
            ident.clear();
            ident.append(c);
            qDebug() << "str: " << handle_ident(c).toString();
            return TOKEN_IDENT;
        }
        else if (c == '=') {
            return '=';
        }
        else if (c == ' ' || c == '\t' || c == '\n') {
            if (c == '\n')
            ++line_no;
            continue;
        }
        else return 0;
    }
    return c;
}

static void process_char()
{
    int c;
    while (1) {
        ident.clear();
        c = handle_comment();
        if (c == TOKEN_IDENT) {
            c = handle_comment();
            if (c == '=') {
                handle_assign();
                break;
                //continue;
            }   //else
            //throw new dBaseError("wrong1 character.");
        }
        else if (c == 0)
        throw new dBaseEndOfProgram;  else
        throw new dBaseError("command invalid form");
    }
}

// --------------------------------
// reset variables for next run ...
// --------------------------------
void reset_program()
{
    line_no  = 1;
    char_pos = 0;
    
    ident       .clear();
    number_ident.clear();
    
    //delete expr_class;
    //ExprClass  * root_expr = new ExprClass;
    //expr_class = root_expr->init();
}

void MainWindow::on_pushButton_clicked()
{
    testAST();
    
    #if 0
    return;
    
    
    srcfile = new QFile(
    QApplication::applicationDirPath() + "/test.code");
    srcfile->open(QIODevice::ReadOnly | QIODevice::Text);
    srcfile->seek(0);

    try {
        reset_program ();
        ident.clear   ();
        process_char  ();
        srcfile->close();
        qDebug() << "ende: " << line_no;
    }
    catch (dBaseEndOfProgram *e) {
        Q_UNUSED(e);
        QMessageBox::information(this,"Information",
        QString("End of Program.\nParsed lines: %1").arg(line_no));

        reset_program ();
        srcfile->close();
        return;
    }
    catch (dBaseError *e) {
        srcfile->close();
        QMessageBox::critical(this,tr("Error"),
        QString("%1\n%2")
        
        .arg(e->m_message)
        .arg("\nLine: %1")
        .arg(line_no));
    }
    catch (...) {
        srcfile->close();
        QMessageBox::critical(this,
        tr("Error"),
        tr("unknown error"));
    }
    #endif
}
