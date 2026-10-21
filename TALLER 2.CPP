#include <iostream>
#include "simlib.h"

enum DistType { EXPONENTIAL, CONSTANT };

struct Configuration {
    int run_id;
    DistType dist_arr;
    DistType dist_s1;
    DistType dist_s2;
};

Configuration current_config;

const double MEAN_ARRIVAL = 10.0;
const double MEAN_SERVICE_1 = 9.0;
const double MEAN_SERVICE_2 = 9.0;
const double SIM_TIME_MAX = 10000.0;

Facility Server1("Servidor Proceso 1");
Facility Server2("Servidor Proceso 2");
Queue Queue1("Cola Proceso 1");
Queue Queue2("Cola Proceso 2");
Stat TotalSystemTime("Tiempo total en el sistema (W)");

class Elemento : public Process {
    void Behavior() override {
        double time_arrive_sys = Time;

        // Proceso 1
        Seize(Server1);
        if (current_config.dist_s1 == EXPONENTIAL) {
            Wait(Exponential(MEAN_SERVICE_1));
        } else {
            Wait(MEAN_SERVICE_1);
        }
        Release(Server1);

        // Proceso 2
        Seize(Server2);
        if (current_config.dist_s2 == EXPONENTIAL) {
            Wait(Exponential(MEAN_SERVICE_2));
        } else {
            Wait(MEAN_SERVICE_2);
        }
        Release(Server2);

        // Registro de estadísticas
        TotalSystemTime(Time - time_arrive_sys);
    }
};

class GeneradorLlegadas : public Event {
    void Behavior() override {
        (new Elemento)->Activate();

        double next_arrival = (current_config.dist_arr == EXPONENTIAL)
                             ? Exponential(MEAN_ARRIVAL)
                             : MEAN_ARRIVAL;

        Activate(Time + next_arrival);
    }
};

void report() {
    // Print imprime directamente en el archivo configurado con SetOutput
    Print("======================================================================\n");
    Print("                      CORRIDA CONFIGURACION %d\n", current_config.run_id);
    Print("======================================================================\n");

    // .Output() sin argumentos escribe al canal de salida activo de SIMLIB
    Queue1.Output();
    Queue2.Output();
    Server1.Output();
    Server2.Output();
    TotalSystemTime.Output();

    Print("======================================================================\n\n\n");
}

int main() {
    // Redirigir la salida global de SIMLIB al archivo de resultados
    SetOutput("simlib_reporte_resultados.txt");

    Configuration configs[4] = {
        {1, EXPONENTIAL, EXPONENTIAL, EXPONENTIAL},
        {2, CONSTANT,    EXPONENTIAL, EXPONENTIAL},
        {3, EXPONENTIAL, CONSTANT,    CONSTANT},
        {4, CONSTANT,    CONSTANT,    CONSTANT}
    };

    for (int i = 0; i < 4; ++i) {
        current_config = configs[i];

        Init(0, SIM_TIME_MAX);

        TotalSystemTime.Clear();
        Queue1.Clear();
        Queue2.Clear();
        Server1.Clear();
        Server2.Clear();

        Server1.SetQueue(Queue1);
        Server2.SetQueue(Queue2);

        (new GeneradorLlegadas)->Activate();

        Run();

        report();
    }

    std::cout << "Simulación completada con éxito. Resultados en 'simlib_reporte_resultados.txt'." << std::endl;
    return 0;
}
