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

class dBaseError {
public:
    dBaseError(QString message) : m_message(message) {}
    QString m_message;
};

QFile * srcfile;
QString buffer;
int     char_pos= 0;
int     line_no = 0;

QString ident;

const int MAX_READ_LINE = 4096;     // maximum of read-in per line
const int TOKEN_IDENT   = 1000;

static int process_char(int ch)
{
    int cflag = 0;
    int c = ch;

    if (((c >= 'a') && (c <= 'z'))
    ||  ((c >= 'A') && (c <= 'Z'))
    ||   (c == '_')) {
        ident.append(ch);
        while (1) {
            if (char_pos >= buffer.size())
            break;
            c = buffer.at(char_pos++).toLatin1();
            if (((c >= '0') && (c <= '9'))
            ||  ((c >= 'a') && (c <= 'z'))
            ||  ((c >= 'A') && (c <= 'Z'))
            ||   (c == '_')) {
                ident.append(c);
                continue;
            }
            else {
                switch (c) {
                case 32:
                case 10:                
                case  9: {
                    if (ident.trimmed().size() > 0) {
                        qDebug() << "id: " << ident;
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
                    
                    else if (c ==  10) break;
                    else if (c ==   9) break;
                    else if (c ==  32) break;
                    
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
        
/*    else if (c ==  10) break;
    else if (c ==   9) break;
    else if (c ==  32) break;*/
        
    return c;
}

void MainWindow::on_pushButton_clicked()
{
    srcfile = new QFile(QString("%1/test.code")
    .arg(qApp->applicationDirPath()));
    srcfile->open(QIODevice::ReadOnly | QIODevice::Text);
    
    int c;
    char *buff_ = new char[MAX_READ_LINE];
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
                    if (char_pos >= buffer.size())
                    break;
                    
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
            }
            else {
                throw new dBaseError(QString("Syntax Error."));
            }
        }
        
        QMessageBox::information(this,"Information",
        QString("parsed lines: %1").arg(line_no));
        
        // --------------------------------
        // reset variables for next run ...
        // --------------------------------
        srcfile->close();
        line_no  = 0;
        char_pos = 0;
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
}
