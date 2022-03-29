/*
cartotype_qt_map_renderer.h

Copyright (C) 2017 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_QT_MAP_RENDERER_H__
#define CARTOTYPE_QT_MAP_RENDERER_H__

#include <QOpenGLFunctions>
#include "cartotype_framework.h"

namespace CartoType
{

class CVectorTileServer;

/** A graphics-accelerated map renderer for use with Qt. */
class CQtMapRenderer
    {
    public:
    CQtMapRenderer(CFramework& aFramework);
    void Init();
    void Draw();

    private:
    CFramework& m_framework;
    std::shared_ptr<CVectorTileServer> m_vector_tile_server;
    };

}

#endif // CARTOTYPE_QT_MAP_RENDERER_H__
