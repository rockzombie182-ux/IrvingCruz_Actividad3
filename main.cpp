#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include "sqlite3.h"

using namespace std;

struct CentroTrabajo {
    string clave;
    string nombre;
    string ciudad;
};

vector<CentroTrabajo> catalogoCentros;

string buscarNombreCentro(const string& clave) {
    for (const auto& c : catalogoCentros) {
        if (c.clave == clave) {
            return c.nombre;
        }
    }
    return "Centro No Encontrado";
}

class Empleado {
    static int contadorEmpleados;
protected:
    int numEmpleado;
    string nombre;
    string apellidoPaterno;
    string apellidoMaterno;
    string fechaNacimiento;
    string rfc;
    string claveCentro;
    string nombreCentro;
    string puesto;
    string descripcionPuesto;
    int esDirectivo;

    string calcularRFC() {
        string rfcGenerado = "";
        if (apellidoPaterno.length() >= 2) {
            rfcGenerado += toupper(apellidoPaterno[0]);
            rfcGenerado += toupper(apellidoPaterno[1]);
        }
        if (!apellidoMaterno.empty()) {
            rfcGenerado += toupper(apellidoMaterno[0]);
        }
        if (!nombre.empty()) {
            rfcGenerado += toupper(nombre[0]);
        }
        if (fechaNacimiento.length() == 10) {
            rfcGenerado += fechaNacimiento.substr(8, 2);
            rfcGenerado += fechaNacimiento.substr(3, 2);
            rfcGenerado += fechaNacimiento.substr(0, 2);
        }
        rfcGenerado += "XXX";
        return rfcGenerado;
    }

public:
    Empleado(string nom, string apPat, string apMat, string fechaNac,
             string claveC, string pto, string descPto, int dir = 0) {
        numEmpleado = ++contadorEmpleados;
        nombre = nom;
        apellidoPaterno = apPat;
        apellidoMaterno = apMat;
        fechaNacimiento = fechaNac;
        claveCentro = claveC;
        nombreCentro = buscarNombreCentro(claveC);
        puesto = pto;
        descripcionPuesto = descPto;
        esDirectivo = dir;
        rfc = calcularRFC();
    }

    virtual ~Empleado() {}

    int getNumEmpleado() const { return numEmpleado; }
    string getNombreCompleto() const {
        return nombre + " " + apellidoPaterno + " " + apellidoMaterno;
    }
    string getFechaNacimiento() const { return fechaNacimiento; }
    string getRFC() const { return rfc; }
    string getClaveCentro() const { return claveCentro; }
    string getNombreCentro() const { return nombreCentro; }
    string getPuesto() const { return puesto; }
    string getDescripcionPuesto() const { return descripcionPuesto; }
    int getEsDirectivo() const { return esDirectivo; }

    virtual string getCentroSupervisa() const { return "N/A"; }
    virtual string getPrestacionCombustible() const { return "No"; }
};

int Empleado::contadorEmpleados = 0;

class Directivo : public Empleado {
private:
    string centroSupervisa;
    bool prestacionCombustible;

public:
    Directivo(string nom, string apPat, string apMat, string fechaNac,
              string claveC, string pto, string descPto,
              string centroSup, bool combustible)
        : Empleado(nom, apPat, apMat, fechaNac, claveC, pto, descPto, 1) {
        centroSupervisa = centroSup;
        prestacionCombustible = combustible;
    }

    string getCentroSupervisa() const override { return centroSupervisa; }
    string getPrestacionCombustible() const override {
        return prestacionCombustible ? "Si" : "No";
    }
};

void imprimirLineaGeneral() {
    cout << "+-----+----------------------------+------------+---------------+------------------------------+--------------------+--------------+" << endl;
}

void imprimirReporteGeneral(const vector<Empleado*>& lista) {
    cout << "\n=== REPORTE GENERAL DE EMPLEADOS Y DIRECTIVOS (EMPRESA UNI) ===" << endl;
    imprimirLineaGeneral();
    cout << "| " << left << setw(3) << "No."
         << " | " << left << setw(26) << "Nombre Completo"
         << " | " << left << setw(10) << "F. Nac."
         << " | " << left << setw(13) << "RFC"
         << " | " << left << setw(28) << "Nombre de Centro"
         << " | " << left << setw(18) << "Desc. Puesto"
         << " | " << left << setw(12) << "Es Directivo" << " |" << endl;
    imprimirLineaGeneral();

    for (const auto& emp : lista) {
        cout << "| " << left << setw(3) << emp->getNumEmpleado()
             << " | " << left << setw(26) << emp->getNombreCompleto()
             << " | " << left << setw(10) << emp->getFechaNacimiento()
             << " | " << left << setw(13) << emp->getRFC()
             << " | " << left << setw(28) << emp->getNombreCentro()
             << " | " << left << setw(18) << emp->getDescripcionPuesto()
             << " | " << left << setw(12) << (emp->getEsDirectivo() == 1 ? "Si" : "No") << " |" << endl;
    }
    imprimirLineaGeneral();
}

void imprimirTablaDirectivos(const vector<Empleado*>& lista) {
    cout << "\n=== CONSULTA SQL: DETALLE DE DIRECTIVOS Y PRESTACIONES ===" << endl;
    cout << "+-----+----------------------------+------------------+----------------------+" << endl;
    cout << "| " << left << setw(3) << "No."
         << " | " << left << setw(26) << "Nombre Directivo"
         << " | " << left << setw(16) << "Centro Supervisa"
         << " | " << left << setw(20) << "Apoyo Combustible" << " |" << endl;
    cout << "+-----+----------------------------+------------------+----------------------+" << endl;

    for (const auto& emp : lista) {
        if (emp->getEsDirectivo() == 1) {
            cout << "| " << left << setw(3) << emp->getNumEmpleado()
                 << " | " << left << setw(26) << emp->getNombreCompleto()
                 << " | " << left << setw(16) << emp->getCentroSupervisa()
                 << " | " << left << setw(20) << emp->getPrestacionCombustible() << " |" << endl;
        }
    }
    cout << "+-----+----------------------------+------------------+----------------------+" << endl;
}

void inicializarBaseDatos(sqlite3* db) {
    const char* sqlSetup =
        "CREATE TABLE IF NOT EXISTS CentrosTrabajo ("
        "  clave TEXT PRIMARY KEY,"
        "  nombre TEXT NOT NULL,"
        "  ciudad TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS Empleados ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  nombre TEXT NOT NULL,"
        "  apellidoPaterno TEXT NOT NULL,"
        "  apellidoMaterno TEXT NOT NULL,"
        "  fechaNacimiento TEXT NOT NULL,"
        "  claveCentro TEXT NOT NULL,"
        "  puesto TEXT NOT NULL,"
        "  descripcionPuesto TEXT NOT NULL,"
        "  esDirectivo INTEGER NOT NULL,"
        "  centroSupervisa TEXT,"
        "  prestacionCombustible INTEGER,"
        "  FOREIGN KEY(claveCentro) REFERENCES CentrosTrabajo(clave)"
        ");"
        "INSERT OR IGNORE INTO CentrosTrabajo VALUES ('000201', 'Tiendas Angel Flores Ropa', 'Culiacan');"
        "INSERT OR IGNORE INTO CentrosTrabajo VALUES ('000202', 'Tiendas Angel Flores Muebles', 'Culiacan');"
        "INSERT OR IGNORE INTO CentrosTrabajo VALUES ('000203', 'Tiendas Angel Flores Cajas', 'Culiacan');"
        "INSERT OR IGNORE INTO CentrosTrabajo VALUES ('049001', 'La Primavera Ropa', 'Culiacan');"
        "INSERT OR IGNORE INTO CentrosTrabajo VALUES ('049002', 'La Primavera Muebles', 'Culiacan');"
        "INSERT OR IGNORE INTO CentrosTrabajo VALUES ('049003', 'La Primavera Cajas', 'Culiacan');";

    char* errMsg = nullptr;
    sqlite3_exec(db, sqlSetup, nullptr, nullptr, &errMsg);
    if (errMsg) {
        cerr << "Error SQL Setup: " << errMsg << endl;
        sqlite3_free(errMsg);
    }

    sqlite3_stmt* stmtCount;
    int total = 0;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM Empleados;", -1, &stmtCount, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmtCount) == SQLITE_ROW) {
            total = sqlite3_column_int(stmtCount, 0);
        }
        sqlite3_finalize(stmtCount);
    }

    if (total == 0) {
        const char* sqlInsert =
            "INSERT INTO Empleados (nombre, apellidoPaterno, apellidoMaterno, fechaNacimiento, claveCentro, puesto, descripcionPuesto, esDirectivo, centroSupervisa, prestacionCombustible) VALUES "
            "('Jesus', 'Vega', 'Castro', '26/03/1988', '000201', 'Asesor', 'Vendedor', 0, NULL, 0),"
            "('Jose', 'Lopez', 'Perez', '01/01/1980', '049003', 'Gerente', 'Gerente', 1, '049003', 1),"
            "('Maria', 'Gomez', 'Ruiz', '15/07/1992', '000202', 'Atencion', 'Cajero', 0, NULL, 0),"
            "('Carlos', 'Mendoza', 'Soto', '10/11/1985', '049001', 'Supervisor', 'Jefe de Piso', 1, '049001', 1),"
            "('Ana', 'Torres', 'Diaz', '22/05/1995', '000203', 'Soporte', 'Auxiliar Cajas', 0, NULL, 0);";
        sqlite3_exec(db, sqlInsert, nullptr, nullptr, &errMsg);
        if (errMsg) {
            cerr << "Error SQL Insert: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }
}

int main() {
    sqlite3* db;
    if (sqlite3_open("UNI.db", &db) != SQLITE_OK) {
        cerr << "Error al abrir la base de datos: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    inicializarBaseDatos(db);

    sqlite3_stmt* stmtCentros;
    if (sqlite3_prepare_v2(db, "SELECT clave, nombre, ciudad FROM CentrosTrabajo;", -1, &stmtCentros, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmtCentros) == SQLITE_ROW) {
            CentroTrabajo c;
            c.clave = reinterpret_cast<const char*>(sqlite3_column_text(stmtCentros, 0));
            c.nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmtCentros, 1));
            c.ciudad = reinterpret_cast<const char*>(sqlite3_column_text(stmtCentros, 2));
            catalogoCentros.push_back(c);
        }
        sqlite3_finalize(stmtCentros);
    }

    vector<Empleado*> plantillaUNI;
    const char* queryEmpleados =
        "SELECT nombre, apellidoPaterno, apellidoMaterno, fechaNacimiento, "
        "claveCentro, puesto, descripcionPuesto, esDirectivo, centroSupervisa, prestacionCombustible "
        "FROM Empleados;";

    sqlite3_stmt* stmtEmp;
    if (sqlite3_prepare_v2(db, queryEmpleados, -1, &stmtEmp, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmtEmp) == SQLITE_ROW) {
            string nom = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 0));
            string apPat = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 1));
            string apMat = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 2));
            string fNac = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 3));
            string cCentro = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 4));
            string pto = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 5));
            string descPto = reinterpret_cast<const char*>(sqlite3_column_text(stmtEmp, 6));
            int esDir = sqlite3_column_int(stmtEmp, 7);

            if (esDir == 1) {
                const unsigned char* supText = sqlite3_column_text(stmtEmp, 8);
                string cSup = supText ? reinterpret_cast<const char*>(supText) : "N/A";
                bool comb = (sqlite3_column_int(stmtEmp, 9) == 1);
                plantillaUNI.push_back(new Directivo(nom, apPat, apMat, fNac, cCentro, pto, descPto, cSup, comb));
            } else {
                plantillaUNI.push_back(new Empleado(nom, apPat, apMat, fNac, cCentro, pto, descPto));
            }
        }
        sqlite3_finalize(stmtEmp);
    }

    sqlite3_close(db);

    imprimirReporteGeneral(plantillaUNI);
    imprimirTablaDirectivos(plantillaUNI);

    for (auto emp : plantillaUNI) {
        delete emp;
    }

    return 0;
}
