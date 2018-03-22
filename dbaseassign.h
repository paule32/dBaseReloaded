#ifndef DBASEASSIGN_H
#define DBASEASSIGN_H

#include <QDebug>
#include "dbasecomando.h"
#include "dbasevisitor.h"

class dBaseAssign: public dBaseComando {
public:
    dBaseAssign() { qDebug() << "dBaseAssign"; }
    virtual void accept(dBaseVisitor &v);
    QString getLName(void) { return QString("testvar"); }
};

#endif // DBASEASSIGN_H
