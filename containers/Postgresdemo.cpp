#include <iostream>
#include <string>
#include "../types.h"

using namespace std;

// =====================================================
// Exploración de 3 tipos de índice en PostgreSQL
//
// 1. B-Tree  — índice por defecto, balanceado
// 2. Hash    — acceso O(1) promedio por igualdad exacta
// 3. GiST    — índice espacial generalizado (base de RTree)
//
// Referencia: https://www.postgresql.org/docs/current/indexes.html
// =====================================================

// ─────────────────────────────────────────────────────
// 1. B-TREE INDEX
//
// Estructura: árbol B+ balanceado con nodos de página.
// Casos de uso: ORDER BY, BETWEEN, <, >, =, >=, <=
// PostgreSQL lo usa por defecto en CREATE INDEX.
// Complejidad de búsqueda: O(log n)
// ─────────────────────────────────────────────────────

void DemoPostgresBTree()
{
    cout << "\n=== INDICE B-TREE (PostgreSQL) ===" << endl;

    cout << "\nSQL para crear tabla y índice B-Tree:" << endl;
    cout << "  CREATE TABLE empleados (" << endl;
    cout << "      id     SERIAL PRIMARY KEY," << endl;
    cout << "      nombre VARCHAR(100)," << endl;
    cout << "      salario NUMERIC" << endl;
    cout << "  );" << endl;
    cout << "  -- B-Tree es el índice por defecto" << endl;
    cout << "  CREATE INDEX idx_salario ON empleados(salario);" << endl;

    cout << "\nConsultas que aprovechan el B-Tree:" << endl;
    cout << "  SELECT * FROM empleados WHERE salario > 3000;" << endl;
    cout << "  SELECT * FROM empleados ORDER BY salario;" << endl;
    cout << "  SELECT * FROM empleados WHERE salario BETWEEN 2000 AND 5000;" << endl;

    cout << "\nCaracteristicas:" << endl;
    cout << "  - Soporta: =, <, >, <=, >=, BETWEEN, LIKE 'abc%'" << endl;
    cout << "  - Mantiene datos ordenados en disco" << endl;
    cout << "  - Busqueda O(log n), insercion O(log n)" << endl;
    cout << "  - Ideal para rangos y orden" << endl;
}

// ─────────────────────────────────────────────────────
// 2. HASH INDEX
//
// Estructura: tabla hash con buckets.
// Casos de uso: solo igualdad exacta (=)
// NO soporta rangos ni ORDER BY.
// Complejidad de búsqueda: O(1) promedio
// ─────────────────────────────────────────────────────

void DemoPostgresHash()
{
    cout << "\n=== INDICE HASH (PostgreSQL) ===" << endl;

    cout << "\nSQL para crear índice Hash:" << endl;
    cout << "  CREATE TABLE sesiones (" << endl;
    cout << "      token  VARCHAR(64) PRIMARY KEY," << endl;
    cout << "      usuario_id INT" << endl;
    cout << "  );" << endl;
    cout << "  CREATE INDEX idx_token ON sesiones USING HASH(token);" << endl;

    cout << "\nConsultas que aprovechan el Hash:" << endl;
    cout << "  SELECT * FROM sesiones WHERE token = 'abc123';" << endl;

    cout << "\nCaracteristicas:" << endl;
    cout << "  - Solo soporta: =" << endl;
    cout << "  - NO soporta: rangos, ORDER BY, LIKE" << endl;
    cout << "  - Busqueda O(1) promedio" << endl;
    cout << "  - Util para claves largas (UUIDs, tokens, hashes)" << endl;
    cout << "  - En PostgreSQL < 10 no era crash-safe (ahora si)" << endl;
}

// ─────────────────────────────────────────────────────
// 3. GIST / RTREE INDEX (Espacial)
//
// Estructura: GiST (Generalized Search Tree) — en la
// práctica implementa R-Tree para datos espaciales.
// Casos de uso: geometría, geografía, rangos superpuestos.
// Requiere extensión PostGIS para geometría completa.
// ─────────────────────────────────────────────────────

void DemoPostgresRTree()
{
    cout << "\n=== INDICE ESPACIAL GiST/RTree (PostgreSQL) ===" << endl;

    cout << "\nSQL para crear índice espacial con PostGIS:" << endl;
    cout << "  -- Requiere: CREATE EXTENSION postgis;" << endl;
    cout << "  CREATE TABLE tiendas (" << endl;
    cout << "      id       SERIAL PRIMARY KEY," << endl;
    cout << "      nombre   VARCHAR(100)," << endl;
    cout << "      ubicacion GEOMETRY(POINT, 4326)" << endl;
    cout << "  );" << endl;
    cout << "  -- GiST implementa R-Tree internamente" << endl;
    cout << "  CREATE INDEX idx_ubicacion ON tiendas USING GIST(ubicacion);" << endl;

    cout << "\nConsultas que aprovechan el GiST/R-Tree:" << endl;
    cout << "  -- Tiendas a menos de 1km de un punto" << endl;
    cout << "  SELECT nombre FROM tiendas" << endl;
    cout << "  WHERE ST_DWithin(ubicacion," << endl;
    cout << "         ST_MakePoint(-77.03, -12.05)::geography, 1000);" << endl;
    cout << "  -- Interseccion de geometrias" << endl;
    cout << "  SELECT * FROM zonas WHERE ST_Intersects(area, :region);" << endl;

    cout << "\nCaracteristicas:" << endl;
    cout << "  - Soporta: &&, @>, <@, ST_Within, ST_Intersects, etc." << endl;
    cout << "  - Organiza objetos en cajas delimitadoras (MBR)" << endl;
    cout << "  - Ideal para consultas de proximidad y geometria" << endl;
    cout << "  - Mas flexible que B-Tree: extensible a tipos custom" << endl;
}

// ─────────────────────────────────────────────────────
// Comparativa de los 3 índices
// ─────────────────────────────────────────────────────

void DemoPostgresComparativa()
{
    cout << "\n=== COMPARATIVA DE INDICES ===" << endl;
    cout << "\n"
         << "  Caracteristica       | B-Tree      | Hash        | GiST/R-Tree\n"
         << "  ---------------------|-------------|-------------|------------\n"
         << "  Igualdad (=)         | Si          | Si (O(1))   | Si\n"
         << "  Rango (<, >, BETWEEN)| Si          | No          | Si (espacial)\n"
         << "  ORDER BY             | Si          | No          | No\n"
         << "  Datos espaciales     | No          | No          | Si\n"
         << "  Complejidad busqueda | O(log n)    | O(1) prom   | O(log n)\n"
         << "  Uso por defecto      | Si          | No          | No\n"
         << "  Extension requerida  | Ninguna     | Ninguna     | PostGIS\n"
         << endl;
}

// ─────────────────────────────────────────────────────
// Entry point de la demo
// ─────────────────────────────────────────────────────

void DemoPostgres()
{
    cout << "\n=====================================" << endl;
    cout << " INDICES EN POSTGRESQL" << endl;
    cout << "=====================================" << endl;

    DemoPostgresBTree();
    DemoPostgresHash();
    DemoPostgresRTree();
    DemoPostgresComparativa();
}