/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KDEVAPI_H_
#define _KDEVAPI_H_

class KDevCore;
class KDevProject;
class KDevVersionControl;
class KDevLanguageSupport;
class KDevMakeFrontend;
class KDevAppFrontend;
class ClassStore;
class QDomDocument;
class KDevPartController;


class KDevApiPrivate;

// 2002-02-08 add ccClassStore - daniel
class KDevApi
{
public:

    KDevApi();
    virtual ~KDevApi();

    virtual KDevPartController *partController() = 0;
    virtual KDevCore *core() = 0;
    virtual ClassStore *classStore() = 0;
    virtual ClassStore *ccClassStore() = 0;

    QDomDocument *projectDom();
    void setProjectDom(QDomDocument *dom);

    KDevProject *project();
    void setProject(KDevProject *project);
   
    KDevMakeFrontend *makeFrontend();
    void setMakeFrontend(KDevMakeFrontend *makeFrontend);

    KDevAppFrontend *appFrontend();
    void setAppFrontend(KDevAppFrontend *appFrontend);
   
    KDevLanguageSupport *languageSupport();
    void setLanguageSupport(KDevLanguageSupport *languageSupport);

    KDevVersionControl *versionControl();
    void setVersionControl(KDevVersionControl *versionControl);


private:
    
    KDevApiPrivate *d;

};

#endif
