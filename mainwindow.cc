#include <cctype>
using namespace std;

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <QString>
#include <QFile>
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
      
QFile * srcfile;
QString buffer;
int     char_pos= 0;
int     line_no = 0;
char  * buff_ = {0};

QString ident;

const int MAX_READ_LINE = 4096;     // maximum of read-in per line
const int TOKEN_IDENT   = 1000;

// internal context's (parser) ...
// ---------------------------------
int current_context             = 0; // default context
int current_context_do          = 0; // keyword: do
int current_context_newcommand  = 0; // mark for command processing

QMap<QString,QVariant> MyParameters;


static void inline check_buffer()
{
    if (char_pos-1 >= buffer.size())
    throw new dBaseError(
    QString("buffer overflow."));
}

static int get_char()
{
    check_buffer();
    return buffer.at(char_pos++).toLatin1();
}
static int process_char(int ch)
{
    int cflag = 0;
    int c = ch;

    if (((c >= 'a') && (c <= 'z'))
    ||  ((c >= 'A') && (c <= 'Z'))
    ||   (c == '_')) {
        ident.append(ch);
        while (1) {
            c = get_char();
            if (((c >= '0') && (c <= '9'))
            ||  ((c >= 'a') && (c <= 'z'))
            ||  ((c >= 'A') && (c <= 'Z'))
            ||   (c == '_')) {
                ident.append(c);
                continue;
            }
            else {
                switch (c) {
                case 10: {
                    qDebug() << "as: " << ident;
                    
                    if (current_context_do == 2)
                    {   qDebug() << "wuitt";
                        ident.clear();
                        while (1) {
                            c = get_char();
                            if (((c >= '0') && (c <= '9'))
                            ||  ((c >= 'a') && (c <= 'z'))
                            ||  ((c >= 'A') && (c <= 'Z'))
                            ||   (c == '_') || (c == '.')) {
                                ident.append(c);
                                continue;
                            }
                            else if (c == 10) {
                            qDebug() << "for22: " << ident;
                                throw new dBaseNewLine;
                                break;
                            }
                            else if (c == 32 || c == 9 || c == 7) {
                                continue;
                            }
                            
                            else if (c == ',') {
                                qDebug() << "kommas: " << ident;
                                ident.clear();
                                continue;
                            }
                            
                            current_context_do = 2;
                            
                            if (c == '&') process_char('&'); else
                            if (c == '*') process_char('*'); else
                            if (c == '/') process_char('/'); else {
                                if (char_pos >= buffer.size())
                                throw new dBaseEndOfProgram;
                                
                                else
                                qDebug() << "1formser: " << ident;
                                break;
                            }
                        }
                    }
                    else
                    throw new dBaseNewLine;
                }
                break;
                
                case 32:
                case  7:
                case  9: {
                    if (ident.trimmed().size() > 0) {
                        ident = ident.toLower();
                        qDebug() << "id: " << ident;
                        if (ident == "do") {
                            qDebug() << "ein dooer";
                            current_context_do = 1;
                            while (1) {
                                c = get_char();
                                if (((c >= '0') && (c <= '9'))
                                ||  ((c >= 'a') && (c <= 'z'))
                                ||  ((c >= 'A') && (c <= 'Z'))
                                ||   (c == '_')) {
                                    ident.append(c);
                                    continue;
                                }
                                else if (c == 10) {
                                    throw new dBaseNewLine;
                                    break;
                                }
                                else if (c == 32 || c == 9 || c == 7) {
                                    qDebug() << "Aformser: " << ident;
                                    break;
                                }
                                
                                else if (c == '&') goto and_goto;
                                else if (c == '*') goto mul_goto;
                                else if (c == '/') goto div_goto;
                                
                                else break;
                            }
                            qDebug() << "Bformser: " << ident;
                        }
                        ident.clear();
                    }
                }
                break;
     
                case ',': {

                    QString s("parameter");
                    if (ident.contains(s))
                    {
                        QString s2 = ident;
                        s2.resize(s.size());
                        qDebug() << "para: " << s2;
                        
                        ident =
                        ident.midRef(s.size()).toString();
                    }
                    qDebug() << "comma: " << ident;
                    ident.clear();
                    c = buffer.at(char_pos++).toLatin1();
                    if (((c >= 'a') && (c <= 'z'))
                    ||  ((c >= 'A') && (c <= 'Z'))
                    ||   (c == '_')) {
                        ident.append(c);
                        continue;
                    }
                    else if (c == '&') goto and_goto;
                    else if (c == '*') goto mul_goto;
                    else if (c == '/') goto div_goto;
                    
                    else if (c ==  10) throw new dBaseNewLine;
                    else if (c ==   9) break;
                    else if (c ==  32) break;
                    
                    else if (c == ',') {
                        qDebug() << "nochn komma";
                    }
                    
                    else {
                    throw new dBaseError(QString("PARAMETER syntax error."));
                    }
                }
                break;
                           
                case '*': {
                mul_goto:
                    c = buffer.at(char_pos).toLatin1();
                    if (c == '*') {
                        // dBase comment 1
                        while (1) {
                            ++char_pos;
                            c = buffer.at(char_pos).toLatin1();
                            if (c == 10)
                            break;
                        }
                    }
                    else throw new dBaseError(
                    QString("syntax error => you mean a comment?"));
                    
                    return TOKEN_IDENT;
                }
                break;
                
                case '&': {
                and_goto:
                    ch = buffer.at(char_pos).toLatin1();
                    if (c == '&') {
                        // dBase comment 1
                        while (1) {
                            ++char_pos;
                            c = buffer.at(char_pos).toLatin1();
                            if (c == 10)
                            break;
                        }
                    }
                    else throw new dBaseError(
                    QString("syntax error => you mean a comment?"));
                    
                    return TOKEN_IDENT;
                }
                break;
                
                case '/': {
                div_goto:
                    c = buffer.at(char_pos++).toLatin1();
                    if (c == '/') {
                        // c++ comment 1
                        while (1) {
                            ++char_pos;
                            c = buffer.at(char_pos).toLatin1();
                            if (c == 10)
                            break;
                        }
                    }
                    else if (c == '*') {
                        while (1) {
                            if (char_pos >= buffer.size())
                            throw new dBaseError("syntax error in comment.");
        
                            c = buffer.at(++char_pos).toLatin1();
                            if (c == '*') {
                                c = buffer.at(++char_pos).toLatin1();
                                if (c == '/') {
                                    cflag = 2;
                                    break;
                                }
                                else {
                                    continue;
                                }
                            }
                            else if (c ==  10) {
                                char_pos = 0;
                                char *buff_ = new char[MAX_READ_LINE];
                                while (!srcfile->atEnd()) {
                                    srcfile->readLine(buff_,MAX_READ_LINE);
                                    buffer.clear();
                                    buffer.append(buff_);
                                    char_pos = -1;
                                    ++line_no;
                                    
                                    if (buffer.size() > 0) {
                                        while (1) {
                                            c = buffer.at(++char_pos).toLatin1();
                                            if (c == '*') {
                                                c = buffer.at(++char_pos).toLatin1();
                                                if (c == 10) {
                                                    char_pos = 0;
                                                    break;
                                                }
                                                else if (c == '/') {
                                                    cflag = 2;
                                                    break;
                                                }
                                            }
                                            else if (c == 10) {
                                                char_pos = 0;
                                                cflag = 3;
                                                break;
                                            }
                                        }
                                        if (cflag == 3)
                                        continue; else
                                        break;
                                    }
                                    else throw new dBaseError(
                                    QString("can not found comment end"));
                                    if (cflag == 2)
                                    break;
                                }
                                if (cflag == 2)
                                break;
                            }
                        }
                        break;
                    }
                    else if (c >= '0' && c <= '9') {
                        --char_pos;
                        return '/';
                    }
                    else if (c >= 'a' && c <= 'z') { --char_pos; return '/'; }
                    else if (c >= 'A' && c <= 'Z') { --char_pos; return '/'; }
                    
                    cflag = 0;
                    return TOKEN_IDENT;
                }
                break;
                
                default: throw new dBaseError(QString("not handled char."));
                break;
                }
            }
        }
    }

    else if (c == '&') goto and_goto;
    else if (c == '*') goto mul_goto;
    else if (c == '/') goto div_goto;
       
    return c;
}

// --------------------------------
// reset variables for next run ...
// --------------------------------
void reset_program()
{
    srcfile->close();
    line_no  = 0;
    char_pos = 0;
    
    current_context = 0;
    current_context_do = 0;
    current_context_newcommand = 0;
    
    delete buff_;
}

void MainWindow::on_pushButton_clicked()
{
    srcfile = new QFile(QString("%1/test.code")
    .arg(qApp->applicationDirPath()));
    srcfile->open(QIODevice::ReadOnly | QIODevice::Text);
    srcfile->seek(0);
    
    int c;
    buff_ = new char[MAX_READ_LINE];
    try {
        while (!srcfile->atEnd()) {
            srcfile->readLine(buff_,MAX_READ_LINE);
            buffer.clear();
            buffer.append(buff_);
            
            ++line_no;
            
            if (buffer.size() > 0) {
                char_pos = 0;
                ident.clear();
                
                while (1) {
                try {
                    if (char_pos >= buffer.size())
                    break;
                    
                    if (current_context_do == 1) {
                        while (1) {
                            c = get_char();
                            if (((c >= '0') && (c <= '9'))
                            ||  ((c >= 'a') && (c <= 'z'))
                            ||  ((c >= 'A') && (c <= 'Z'))
                            ||   (c == '_')) {
                                ident.append(c);
                                continue;
                            }
                            else if (c == 10) {
                                current_context_do = 2;
                                c = get_char( );
                                process_char(c);
                                qDebug() << "for22: " << ident;
                                throw new dBaseNewLine;
                                break;
                            }
                            else if (c == 32 || c == 9 || c == 7) {
                                current_context_do = 2;
                                qDebug() << "Dformser: " << ident;
                                ident.clear();
                                break;
                            }
                            
                            current_context_do = 2;
                            
                            if (c == '&') process_char('&'); else
                            if (c == '*') process_char('*'); else
                            if (c == '/') process_char('/'); else {
                                qDebug() << "Cformser: " << ident;
                                break;
                            }
                        }
                        //current_context_do = 0;
                    }
                    
                    c = buffer.at(char_pos++).toLatin1();
                    c = process_char(c);
                    if (ident.trimmed().size() > 0) {
                        qDebug() << "Informationa: " << ident;
                        if (ident == "parameter") {
                            qDebug() << "parameter token found";
                        }   ident.clear();
                    }
                    else if (c == '/') {
                        ident.clear();
                        qDebug() << "ein comment";
                    }
                    else if (c == 'e') {
                        qDebug() << "ein e";
                    }
                }
                catch (dBaseEndOfProgram *e) {
                    Q_UNUSED(e)
                    QMessageBox::information(this,"Information",
                    QString("End of Program.\nParsed lines: %1").arg(line_no));
                    reset_program();
                    return;
                }
                catch (dBaseNewLine *e) {
                    Q_UNUSED(e)
                    qDebug() << "new line: " << line_no;
                    break;
                }   }
            }
            else {
                throw new dBaseError(QString("Syntax Error."));
            }
        }
        
        QMessageBox::information(this,"Information",
        QString("parsed lines: %1").arg(line_no));
    }
    catch (dBaseError *e) {
        QMessageBox::critical(this,tr("Error"),
        QString("%1\n%2")
            .arg(e->m_message)
            .arg("\nLine: %1")
            .arg(line_no));
    }
    catch (...) {
        QMessageBox::critical(this,tr("Error"),tr("unknown error"));
    }
    
    // ---------------------
    // reset for nex run ...
    // ---------------------
    reset_program();
}
