#include <iostream>
#include <cctype>
using namespace std;

#include <string>
#include <cstdlib>
#include <cxxabi.h>

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

enum enumMathCommand {
    ncMUL
};
enum enumClassType {
    ctAssign,
    ctAssignADD,
    ctAssignSUB,
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

const int TOKEN_IS_NUMBER     = 2;
const int TOKEN_IS_LETTER     = 3;

int line_no;
QFile * srcfile = nullptr;

class dBaseParser {
public:
    virtual void start() {
        QString codestart = "  ssds =  44\n    ** /* dsdas dass d asd d */ xxx = 1223"
        "\n\n// dsgfdsdsg && gfegefg xcyxcc = 111\n"
        "sss = 1212"
        " car = 133.122";
        
        codestart = codestart.replace("\n",
        QString("\n# line %1\n").arg(++line_no));
        
        QStringList codelineList;
        QString     codeline;
        int rc = 0;
        int k = 0;

        QString substr = "# line 1";
        line_no = 1;
        while (codestart.indexOf(substr) != -1) {        
            codestart.replace(codestart.indexOf(substr), substr.length(),
            QString("# line %1").arg(line_no));
            ++line_no;
        }
        line_no = 1;
        
        codelineList = codestart.split(QRegExp("[\n]+"));
        
        for (int i = 0; i < codelineList.count(); i++)
        {
            codeline = codelineList.at(i);
            codeline = codeline.trimmed();
            
            if (codeline.at(0) == '#') {
                qDebug() << codeline;
                continue;
            }   else
            if (codeline.at(0) == '/'
            &&  codeline.at(1) == '/') {
                qDebug() << "skip: " << codeline;
                continue;
            }   else
            if (codeline.at(0) == '*'
            &&  codeline.at(1) == '*') {
                qDebug() << "skip: " << codeline;
                continue;
            }   else
            if (codeline.at(0) == '&'
            &&  codeline.at(1) == '&') {
                qDebug() << "skip: " << codeline;
                continue;
            }   else
            if (codeline.at(0).isLetter()) {
                qDebug() << "letter: " << codeline;
                continue;
            }   else
            if (codeline.at(0).isDigit()) {
                qDebug() << "number: " << codeline;
                continue;
            }   else  {
                rc = 1;
                for (int j = 0; j < codeline.length(); j++)
                {
                    rc = 1;
                    k  = j;
                    if (codeline.at(k) == ' ') {
                        while (1) {
                            if (codeline.at(++k) == ' ') {
                                ++rc;
                                continue;
                            }
                            break;
                        }
                        codeline =
                        codeline.remove(0,rc);
                        qDebug() << codeline;
                        break;
                    }
                }
            }
        }
        
        qDebug()  << "==> " << codelineList;

        qDebug() << "===================";
        qDebug() << "parser start debase";
        
        qDebug() << "ende";
    }

private:
};

template <class T>
class Parser {
public:
    QString getParserClassName()
    {
        int status;
        std::string tname = typeid(T).name();
        char *demangled_name = abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
        if(status == 0) {
            tname = demangled_name;
            std::free(demangled_name);
        }   
        return QString(tname.c_str());
    }
    void start() {
        QString pc = getParserClassName();
        if (pc == "dBaseParser") {
            dBaseParser * pc = new
            dBaseParser ; pc->start();
        }
    }
};

void testParser()
{
    Parser<dBaseParser> dbasep;
    dbasep.start();
}

void dBaseAssign   ::accept(dBaseVisitor &v) { v.visit(this); }
void dBaseADDnumber::accept(dBaseVisitor &v) { v.visit(this); }

void testAST()
{
    testParser();
    return;
    
    QVector<dBaseComando *> cmds;
    
    dBaseComando * cmd1 = new dBaseAssign   ; cmds.append(cmd1);
    dBaseComando * cmd2 = new dBaseADDnumber; cmds.append(cmd2);
    
    dBaseDownVisitor down;
    for (int i = 0 ; i < cmds.count(); i++) {
        cmds.at(i)->accept(down);
    }
}


// --------------------------------
// reset variables for next run ...
// --------------------------------
void reset_program()
{
    line_no = 0;
}

void MainWindow::on_pushButton_clicked()
{
    srcfile = new QFile(
    QApplication::applicationDirPath() + "/test.code");
    srcfile->open(QIODevice::ReadOnly | QIODevice::Text);
    srcfile->seek(0);

    try {
        reset_program ();
        testAST();
        srcfile->close();
        qDebug() << "ende: " << line_no;
    }
    catch (dBaseEndOfProgram *e) {
        Q_UNUSED(e);
        QMessageBox::information(this,"Information",
        QString("End of Program.\nParsed lines: %1").arg(line_no));

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
}
