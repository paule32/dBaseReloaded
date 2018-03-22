#ifndef DBASEDOWNVISITOR_H
#define DBASEDOWNVISITOR_H

#include "dbasevisitor.h"
#include "dbaseassign.h"
#include "dbaseaddnumber.h"

#include <QDebug>

class dBaseDownVisitor: public dBaseVisitor {
public:
    dBaseDownVisitor() { qDebug() << "dBaseDownVisitor"; }
    virtual void visit(dBaseAssign *e) {
        qDebug() << "Down on: " << e->getLName();
    }
    virtual void visit(dBaseADDnumber *e) {
        qDebug() << "Down on: " << e->getNumber();
    }
    virtual void visit(float *e) {
        qDebug() << "Down on: " << e;
    }
};

#endif // DBASEDOWNVISITOR_H
