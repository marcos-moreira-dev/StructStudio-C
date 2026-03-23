/*
 * StructStudio C
 * --------------
 * Educational theory summaries for structures and algorithms.
 *
 * This catalog stays in the core so the educational content can evolve
 * independently from the window layout. The UI only asks for a contextual
 * summary and renders it inside the right-side panel.
 */

#include "api_internal.h"

#include <stdio.h>
#include <string.h>

static void ss_theory_append_text(char *buffer, size_t capacity, const char *text)
{
    size_t length;

    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    length = strlen(buffer);
    if (length >= capacity - 1) {
        return;
    }

    ss_str_copy(buffer + length, capacity - length, text);
}

static void ss_theory_append_line(char *buffer, size_t capacity, const char *text)
{
    if (buffer == NULL || capacity == 0 || text == NULL) {
        return;
    }

    if (buffer[0] != '\0') {
        ss_theory_append_text(buffer, capacity, "\r\n");
    }
    ss_theory_append_text(buffer, capacity, text);
}

static void ss_theory_append_section(char *buffer, size_t capacity, const char *title)
{
    ss_theory_append_line(buffer, capacity, "");
    ss_theory_append_line(buffer, capacity, title);
    ss_theory_append_line(buffer, capacity, "----------------------------------------");
}

static const char *ss_theory_family_label(SsFamily family)
{
    switch (family) {
        case SS_FAMILY_VECTOR:
            return "Estructura lineal indexada";
        case SS_FAMILY_LIST:
            return "Estructura lineal enlazada";
        case SS_FAMILY_STACK:
            return "Estructura lineal LIFO";
        case SS_FAMILY_QUEUE:
            return "Estructura lineal FIFO/priorizada";
        case SS_FAMILY_TREE:
            return "Estructura jerarquica";
        case SS_FAMILY_HEAP:
            return "Arbol de prioridad";
        case SS_FAMILY_SET:
            return "Coleccion sin duplicados";
        case SS_FAMILY_MAP:
            return "Relacion clave-valor";
        case SS_FAMILY_GRAPH:
            return "Red de vertices y aristas";
        default:
            return "TDA visual";
    }
}

static void ss_theory_append_structure_summary(const SsStructure *structure, char *buffer, size_t capacity)
{
    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            ss_theory_append_line(buffer, capacity, "Concepto base: un vector es una coleccion indexada con acceso directo por posicion.");
            ss_theory_append_line(buffer, capacity, "Idea clave: leer o escribir una celda concreta suele ser O(1), pero insertar en medio puede requerir desplazamientos.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: cada bloque representa una posicion y el indice visible ayuda a relacionar la teoria con la memoria lineal.");
            break;
        case SS_VARIANT_SINGLY_LINKED_LIST:
            ss_theory_append_line(buffer, capacity, "Concepto base: lista enlazada simple. Cada nodo conoce su dato y el siguiente nodo.");
            ss_theory_append_line(buffer, capacity, "Idea clave: insertar al inicio es barato; buscar por posicion requiere recorrer desde la cabecera.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: las flechas muestran el enlace de siguiente para visualizar el recorrido secuencial.");
            break;
        case SS_VARIANT_DOUBLY_LINKED_LIST:
            ss_theory_append_line(buffer, capacity, "Concepto base: lista doblemente enlazada. Cada nodo apunta al siguiente y al anterior.");
            ss_theory_append_line(buffer, capacity, "Idea clave: mejora recorridos y eliminaciones locales porque el nodo conoce ambos vecinos.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: las conexiones ayudan a distinguir la bidireccionalidad del enlace.");
            break;
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
            ss_theory_append_line(buffer, capacity, "Concepto base: lista enlazada simple circular. El ultimo nodo apunta de vuelta al primero.");
            ss_theory_append_line(buffer, capacity, "Idea clave: no existe un NULL final; el recorrido debe detenerse por conteo o por regreso a la cabecera.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: la forma circular ayuda a entender por que la estructura puede reutilizar el recorrido.");
            break;
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            ss_theory_append_line(buffer, capacity, "Concepto base: lista doblemente enlazada circular. Cada nodo mantiene anterior y siguiente dentro de un ciclo cerrado.");
            ss_theory_append_line(buffer, capacity, "Idea clave: combina navegacion en ambos sentidos con estructura circular.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: las conexiones de ambos sentidos muestran la topologia cerrada del TDA.");
            break;
        case SS_VARIANT_STACK:
            ss_theory_append_line(buffer, capacity, "Concepto base: pila (stack). Sigue la politica LIFO: el ultimo en entrar es el primero en salir.");
            ss_theory_append_line(buffer, capacity, "Operaciones tipicas: push, pop y consulta de top.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: la marca TOP indica el extremo operativo de la pila.");
            break;
        case SS_VARIANT_QUEUE:
            ss_theory_append_line(buffer, capacity, "Concepto base: cola (queue). Sigue la politica FIFO: el primero en entrar es el primero en salir.");
            ss_theory_append_line(buffer, capacity, "Operaciones tipicas: enqueue en REAR y dequeue desde FRONT.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: las marcas FRONT y REAR refuerzan la direccion natural del flujo.");
            break;
        case SS_VARIANT_CIRCULAR_QUEUE:
            ss_theory_append_line(buffer, capacity, "Concepto base: cola circular. Reutiliza posiciones de memoria cuando el frente avanza.");
            ss_theory_append_line(buffer, capacity, "Idea clave: evita desperdiciar espacio libre al principio de la cola.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: se conserva la logica FIFO, pero el modelo explica mejor el reaprovechamiento de posiciones.");
            break;
        case SS_VARIANT_PRIORITY_QUEUE:
            ss_theory_append_line(buffer, capacity, "Concepto base: cola de prioridad. La salida depende de la prioridad, no solo del orden de llegada.");
            ss_theory_append_line(buffer, capacity, "Idea clave: el elemento con prioridad dominante debe quedar accesible antes que el resto.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: el campo Numero representa la prioridad educativa asociada al nodo.");
            break;
        case SS_VARIANT_BINARY_TREE:
            ss_theory_append_line(buffer, capacity, "Concepto base: arbol binario. Cada nodo puede tener hasta dos hijos: izquierdo y derecho.");
            ss_theory_append_line(buffer, capacity, "Idea clave: la jerarquia se interpreta desde la raiz hacia abajo; no impone orden por valor.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: los nodos redondos y la marca ROOT ayudan a leer niveles y parentesco.");
            break;
        case SS_VARIANT_BST:
            ss_theory_append_line(buffer, capacity, "Concepto base: BST (Binary Search Tree). Los valores menores van a la izquierda y los mayores a la derecha.");
            ss_theory_append_line(buffer, capacity, "Idea clave: esa invariante permite busquedas, inserciones y borrados guiados por comparacion.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: el auto-layout y los recorridos muestran como el orden logico se convierte en forma visual.");
            break;
        case SS_VARIANT_AVL:
            ss_theory_append_line(buffer, capacity, "Concepto base: arbol AVL. Es un BST auto-balanceado.");
            ss_theory_append_line(buffer, capacity, "Idea clave: mantiene acotada la diferencia de alturas entre subarboles para conservar eficiencia.");
            ss_theory_append_line(buffer, capacity, "Factor de equilibrio (FE): altura(izquierdo) - altura(derecho). Un nodo esta balanceado cuando FE pertenece a {-1, 0, 1}.");
            ss_theory_append_line(buffer, capacity, "Rotaciones clasicas: LL y RR usan rotacion simple; LR y RL usan una rotacion doble para restaurar equilibrio sin perder el orden del BST.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: la forma del arbol ayuda a comparar teoria de balance con la disposicion resultante y las rotaciones pueden estudiarse sobre el nodo pivote.");
            break;
        case SS_VARIANT_HEAP:
            ss_theory_append_line(buffer, capacity, "Concepto base: heap. Es un arbol casi completo que cumple una relacion de prioridad entre padre e hijos.");
            ss_theory_append_line(buffer, capacity, "Idea clave: en max-heap el padre domina; en min-heap el padre es el menor.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: el arbol se usa para explicar la jerarquia, no solo el arreglo interno.");
            break;
        case SS_VARIANT_SET:
            ss_theory_append_line(buffer, capacity, "Concepto base: set. Modela un conjunto de valores unicos.");
            ss_theory_append_line(buffer, capacity, "Idea clave: la pertenencia importa mas que el orden; duplicados violan la semantica del TDA.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: los nodos redondos y la validacion visual enfatizan la unicidad.");
            break;
        case SS_VARIANT_MAP:
            ss_theory_append_line(buffer, capacity, "Concepto base: map. Relaciona una clave unica con un valor asociado.");
            ss_theory_append_line(buffer, capacity, "Idea clave: la operacion principal es recuperar el valor a partir de la clave.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: la tarjeta dividida muestra explicitamente clave y valor para reforzar el modelo.");
            break;
        case SS_VARIANT_DIRECTED_GRAPH:
            ss_theory_append_line(buffer, capacity, "Concepto base: grafo dirigido. Las relaciones tienen sentido u -> v.");
            ss_theory_append_line(buffer, capacity, "Idea clave: no siempre existe el camino inverso; cada arco tiene orientacion.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: las flechas permiten leer dependencia, flujo o precedencia.");
            ss_theory_append_line(buffer, capacity, "Recorridos derivados: BFS y DFS generan arboles de exploracion que muestran como se descubren los vertices desde un origen.");
            break;
        case SS_VARIANT_UNDIRECTED_GRAPH:
            ss_theory_append_line(buffer, capacity, "Concepto base: grafo no dirigido. La arista conecta vertices sin imponer sentido.");
            ss_theory_append_line(buffer, capacity, "Idea clave: la conectividad es simetrica entre los extremos de la arista.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: una sola conexion representa la relacion mutua.");
            ss_theory_append_line(buffer, capacity, "Recorridos derivados: un BFS o DFS puede materializarse como arbol de recorrido para estudiar conectividad y niveles.");
            break;
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
            ss_theory_append_line(buffer, capacity, "Concepto base: grafo dirigido ponderado. Cada arco tiene direccion y peso.");
            ss_theory_append_line(buffer, capacity, "Idea clave: el peso puede representar costo, tiempo o distancia.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: el badge de peso ayuda a unir teoria de caminos con lectura del canvas.");
            ss_theory_append_line(buffer, capacity, "Algoritmos tipicos: BFS/DFS siguen siendo utiles para exploracion; Dijkstra aparece cuando interesa costo minimo desde un origen.");
            break;
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            ss_theory_append_line(buffer, capacity, "Concepto base: grafo no dirigido ponderado. Cada arista tiene un peso comun para ambos extremos.");
            ss_theory_append_line(buffer, capacity, "Idea clave: es el escenario clasico para caminos minimos simples y arboles de expansion minima.");
            ss_theory_append_line(buffer, capacity, "En StructStudio: el peso visible ayuda a razonar Prim, Kruskal y comparaciones de costo.");
            ss_theory_append_line(buffer, capacity, "Arboles del grafo: Prim y Kruskal producen un arbol de expansion minima cuando el grafo es conexo, no dirigido y ponderado.");
            break;
        default:
            ss_theory_append_line(buffer, capacity, "Concepto base: la estructura activa representa un TDA visualizado en 2D.");
            break;
    }
}

static void ss_theory_append_variants_summary(const SsStructure *structure, char *buffer, size_t capacity)
{
    char line[160];

    snprintf(line, sizeof(line), "Familia: %s.", ss_theory_family_label(structure->family));
    ss_theory_append_line(buffer, capacity, line);

    switch (structure->variant) {
        case SS_VARIANT_VECTOR:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: en este proyecto el vector aparece como referencia base para estructuras lineales indexadas.");
            ss_theory_append_line(buffer, capacity, "Comparacion util: frente a listas, pilas o colas, el vector gana acceso directo pero suele perder flexibilidad al insertar o borrar en posiciones intermedias.");
            break;
        case SS_VARIANT_SINGLY_LINKED_LIST:
        case SS_VARIANT_DOUBLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
        case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: Lista simple, Lista doble, Lista circular simple y Lista circular doble.");
            ss_theory_append_line(buffer, capacity, "La diferencia conceptual esta en cuantas referencias guarda cada nodo y en como se detiene el recorrido: por NULL o por retorno al inicio.");
            break;
        case SS_VARIANT_STACK:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: la pila comparte familia con otros TDAs lineales, pero su rasgo distintivo es la disciplina LIFO.");
            ss_theory_append_line(buffer, capacity, "Comparacion util: mientras Queue ordena por llegada y Priority Queue por prioridad, Stack privilegia el elemento mas reciente.");
            break;
        case SS_VARIANT_QUEUE:
        case SS_VARIANT_CIRCULAR_QUEUE:
        case SS_VARIANT_PRIORITY_QUEUE:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: Queue, Circular Queue y Priority Queue.");
            ss_theory_append_line(buffer, capacity, "Todas administran entradas y salidas, pero cambia la politica: FIFO clasico, reutilizacion circular o prioridad del elemento.");
            break;
        case SS_VARIANT_BINARY_TREE:
        case SS_VARIANT_BST:
        case SS_VARIANT_AVL:
        case SS_VARIANT_HEAP:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: Arbol binario, BST, AVL y Heap.");
            ss_theory_append_line(buffer, capacity, "Un BST agrega orden por valor, AVL agrega balance automatico y Heap prioriza la relacion padre-hijo sin exigir orden total entre hermanos.");
            break;
        case SS_VARIANT_SET:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: Set se parece a un contenedor de busqueda, pero su semantica central es la unicidad del elemento.");
            ss_theory_append_line(buffer, capacity, "Comparacion util: frente a Map no almacena pares clave-valor; frente a listas o vectores, no deberia admitir duplicados.");
            break;
        case SS_VARIANT_MAP:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: Map comparte ideas con Set, pero cada clave se asocia a un valor.");
            ss_theory_append_line(buffer, capacity, "Comparacion util: la unicidad vive en la clave; actualizar el valor no deberia crear una nueva entrada logica.");
            break;
        case SS_VARIANT_DIRECTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_GRAPH:
        case SS_VARIANT_DIRECTED_WEIGHTED_GRAPH:
        case SS_VARIANT_UNDIRECTED_WEIGHTED_GRAPH:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: Grafo dirigido, Grafo no dirigido, Grafo dirigido ponderado y Grafo no dirigido ponderado.");
            ss_theory_append_line(buffer, capacity, "Direccion y peso cambian la interpretacion del modelo y determinan que algoritmos son validos o pedagogicamente utiles.");
            break;
        default:
            ss_theory_append_line(buffer, capacity, "Variantes relacionadas: el TDA activo se estudia dentro de su misma familia semantica.");
            break;
    }
}

static void ss_theory_append_analysis_outline(SsAnalysisKind kind, char *buffer, size_t capacity)
{
    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
            ss_theory_append_line(buffer, capacity, "Preorden: visita raiz, luego subarbol izquierdo y despues derecho. Sirve para explicar expansion jerarquica desde la raiz.");
            break;
        case SS_ANALYSIS_TREE_INORDER:
            ss_theory_append_line(buffer, capacity, "Inorden: visita izquierdo, raiz y derecho. En BST produce una lectura ordenada de menor a mayor.");
            break;
        case SS_ANALYSIS_TREE_POSTORDER:
            ss_theory_append_line(buffer, capacity, "Postorden: visita hijos antes de la raiz. Es util para pensar liberacion, evaluacion o agregacion desde hojas.");
            break;
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            ss_theory_append_line(buffer, capacity, "Por niveles: explora la estructura capa por capa usando una cola. Hace visible la profundidad real del arbol.");
            break;
        case SS_ANALYSIS_GRAPH_BFS:
            ss_theory_append_line(buffer, capacity, "BFS: recorre por capas de distancia topologica usando una cola. Es la base para caminos minimos sin pesos.");
            ss_theory_append_line(buffer, capacity, "Resultado derivable: el arbol BFS conserva el primer padre que descubre a cada vertice.");
            break;
        case SS_ANALYSIS_GRAPH_DFS:
            ss_theory_append_line(buffer, capacity, "DFS: profundiza todo lo posible antes de retroceder. Ayuda a estudiar exploracion exhaustiva, componentes y ciclos.");
            ss_theory_append_line(buffer, capacity, "Resultado derivable: el arbol DFS expone la cadena de descubrimiento y el retroceso implicito.");
            break;
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            ss_theory_append_line(buffer, capacity, "Dijkstra: calcula caminos minimos desde un origen en grafos ponderados con pesos no negativos mediante relajaciones.");
            break;
        case SS_ANALYSIS_GRAPH_FLOYD_WARSHALL:
            ss_theory_append_line(buffer, capacity, "Floyd-Warshall: calcula caminos minimos entre todos los pares de vertices usando vertices intermedios.");
            break;
        case SS_ANALYSIS_GRAPH_PRIM:
            ss_theory_append_line(buffer, capacity, "Prim: construye un arbol de expansion minima agregando la arista mas barata que conecta un nuevo vertice.");
            ss_theory_append_line(buffer, capacity, "Requisitos: grafo no dirigido, ponderado y conexo.");
            break;
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            ss_theory_append_line(buffer, capacity, "Kruskal: ordena aristas por peso y acepta solo las que no forman ciclo para construir el arbol minimo.");
            ss_theory_append_line(buffer, capacity, "Requisitos: grafo no dirigido, ponderado y conexo; suele apoyarse en conjuntos disjuntos.");
            break;
        case SS_ANALYSIS_NONE:
        default:
            break;
    }
}

static void ss_theory_append_analysis_details(SsAnalysisKind kind, char *buffer, size_t capacity)
{
    switch (kind) {
        case SS_ANALYSIS_TREE_PREORDER:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Preorden.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: raiz -> izquierdo -> derecho.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: permite seguir la expansion del problema desde el nodo padre hacia sus descendientes.");
            break;
        case SS_ANALYSIS_TREE_INORDER:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Inorden.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: izquierdo -> raiz -> derecho.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: en BST evidencia por que la invariante de orden produce una salida creciente.");
            break;
        case SS_ANALYSIS_TREE_POSTORDER:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Postorden.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: izquierdo -> derecho -> raiz.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: muestra por que muchas operaciones dependen de resolver primero los hijos.");
            break;
        case SS_ANALYSIS_TREE_LEVEL_ORDER:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: recorrido por niveles.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: nivel 0, luego nivel 1, luego nivel 2 y asi sucesivamente.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: conecta muy bien la forma visual del canvas con el uso de una cola auxiliar.");
            break;
        case SS_ANALYSIS_GRAPH_BFS:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: BFS.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: primero vecinos inmediatos, luego vecinos de esos vecinos.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: ilustra exploracion por capas y la nocion de distancia en aristas.");
            ss_theory_append_line(buffer, capacity, "Arbol asociado: cada vertice conserva el primer padre que lo descubrio; por eso el arbol resultante ordena la exploracion por niveles.");
            break;
        case SS_ANALYSIS_GRAPH_DFS:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: DFS.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: seguir un camino profundo y retroceder solo cuando no quedan salidas nuevas.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: explica backtracking, pila implicita y estructura de bosque de exploracion.");
            ss_theory_append_line(buffer, capacity, "Arbol asociado: cada arista de descubrimiento entra al arbol DFS; las demas ayudan a discutir ciclos o retrocesos.");
            break;
        case SS_ANALYSIS_GRAPH_DIJKSTRA:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Dijkstra.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: elegir el vertice con menor distancia provisional y relajar sus salidas.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: permite relacionar pesos visibles con mejoras de costo paso a paso.");
            break;
        case SS_ANALYSIS_GRAPH_FLOYD_WARSHALL:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Floyd-Warshall.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: probar si pasar por un vertice intermedio mejora la distancia entre cada par.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: muestra una vision matricial global, distinta del enfoque desde un origen.");
            break;
        case SS_ANALYSIS_GRAPH_PRIM:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Prim.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: crecer un subarbol ya conectado usando siempre la arista externa mas barata.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: ayuda a comparar conectividad local versus costo total acumulado.");
            ss_theory_append_line(buffer, capacity, "Arbol asociado: el resultado es un arbol de expansion minima orientado pedagogicamente desde el origen elegido.");
            break;
        case SS_ANALYSIS_GRAPH_KRUSKAL:
            ss_theory_append_line(buffer, capacity, "Analisis destacado: Kruskal.");
            ss_theory_append_line(buffer, capacity, "Secuencia mental: ordenar aristas y aceptar solo las que no unan vertices ya conectados por otro camino.");
            ss_theory_append_line(buffer, capacity, "Valor didactico: expone bien la idea de conjuntos disjuntos y deteccion temprana de ciclos.");
            ss_theory_append_line(buffer, capacity, "Arbol asociado: el MST final puede orientarse desde una raiz para dibujarlo como arbol, aunque conceptualmente siga siendo un subgrafo no dirigido.");
            break;
        case SS_ANALYSIS_NONE:
        default:
            break;
    }
}

static void ss_theory_append_available_analyses(
    const SsStructure *structure,
    SsAnalysisKind selected_kind,
    char *buffer,
    size_t capacity)
{
    SsAnalysisKind items[8];
    size_t count = ss_analysis_kinds_for_variant(structure->variant, items, sizeof(items) / sizeof(items[0]));

    if (count > 0) {
        ss_theory_append_line(buffer, capacity, "Recorridos y algoritmos disponibles en StructStudio para la variante activa:");
        for (size_t index = 0; index < count; ++index) {
            ss_theory_append_analysis_outline(items[index], buffer, capacity);
        }
    } else {
        ss_theory_append_line(buffer, capacity, "Recorridos conceptuales para estudiar esta estructura:");
        switch (structure->variant) {
            case SS_VARIANT_VECTOR:
                ss_theory_append_line(buffer, capacity, "Recorrido lineal: avanzar indice por indice de izquierda a derecha para estudiar acceso secuencial o busqueda simple.");
                ss_theory_append_line(buffer, capacity, "Comparacion teorica: si el vector esta ordenado, puede discutirse busqueda binaria aunque no sea playback interactivo del editor.");
                break;
            case SS_VARIANT_SINGLY_LINKED_LIST:
            case SS_VARIANT_DOUBLY_LINKED_LIST:
            case SS_VARIANT_CIRCULAR_SINGLY_LINKED_LIST:
            case SS_VARIANT_CIRCULAR_DOUBLY_LINKED_LIST:
                ss_theory_append_line(buffer, capacity, "Recorrido por enlaces: partir desde la cabecera y seguir siguiente; en variantes dobles tambien puede razonarse recorrido inverso.");
                ss_theory_append_line(buffer, capacity, "Condicion de parada: en variantes no circulares termina en NULL; en circulares termina al volver al nodo inicial o al contar n nodos.");
                break;
            case SS_VARIANT_STACK:
                ss_theory_append_line(buffer, capacity, "Inspeccion teorica: leer de TOP hacia abajo para entender el orden LIFO, aunque el TDA privilegia push y pop por encima de un recorrido formal.");
                break;
            case SS_VARIANT_QUEUE:
            case SS_VARIANT_CIRCULAR_QUEUE:
                ss_theory_append_line(buffer, capacity, "Inspeccion teorica: recorrer de FRONT hacia REAR para estudiar el orden FIFO.");
                ss_theory_append_line(buffer, capacity, "En la variante circular el recorrido conceptual debe respetar el wrap-around del indice.");
                break;
            case SS_VARIANT_PRIORITY_QUEUE:
                ss_theory_append_line(buffer, capacity, "Inspeccion teorica: observar primero el elemento de prioridad dominante y luego el resto segun la politica elegida.");
                ss_theory_append_line(buffer, capacity, "Si se implementa con heap, la lectura por niveles ayuda a entender por que el maximo o minimo queda arriba.");
                break;
            case SS_VARIANT_SET:
                ss_theory_append_line(buffer, capacity, "Busqueda de pertenencia: la pregunta central es si un valor pertenece o no al conjunto.");
                ss_theory_append_line(buffer, capacity, "Recorrido educativo: revisar elementos uno a uno ayuda a contrastar unicidad frente a contenedores con duplicados.");
                break;
            case SS_VARIANT_MAP:
                ss_theory_append_line(buffer, capacity, "Busqueda por clave: el recorrido teorico sigue siendo encontrar la clave correcta antes de devolver el valor asociado.");
                ss_theory_append_line(buffer, capacity, "Comparacion util: a diferencia de Set, aqui cada visita importa por la relacion clave -> valor.");
                break;
            default:
                ss_theory_append_line(buffer, capacity, "La estructura activa no expone un playback especifico; el aprendizaje se centra en operaciones, invariantes y disposicion visual.");
                break;
        }
    }

    if (selected_kind != SS_ANALYSIS_NONE) {
        ss_theory_append_section(buffer, capacity, "Algoritmo destacado ahora");
        ss_theory_append_analysis_details(selected_kind, buffer, capacity);
        if (ss_analysis_supports_playback(selected_kind)) {
            ss_theory_append_line(buffer, capacity, "En StructStudio: puede prepararse un recorrido guiado para observar cada visita, descubrimiento o relajacion.");
        } else if (ss_analysis_supports_tree_generation(selected_kind)) {
            ss_theory_append_line(buffer, capacity, "En StructStudio: puede materializarse una nueva estructura derivada para estudiar el arbol producido por el algoritmo.");
        } else {
            ss_theory_append_line(buffer, capacity, "En StructStudio: este algoritmo se resume en texto porque su explicacion completa necesita una vista global o matricial.");
        }
    }
}

int ss_build_theory_summary(
    const SsStructure *structure,
    SsAnalysisKind kind,
    char *buffer,
    size_t capacity,
    SsError *error)
{
    char line[192];
    const char *suffix = "";

    if (buffer == NULL || capacity == 0) {
        ss_error_set(error, SS_ERROR_ARGUMENT, "Buffer de teoria invalido.");
        return 0;
    }

    buffer[0] = '\0';
    if (structure == NULL) {
        ss_error_set(error, SS_ERROR_INVALID_STATE, "No hay estructura activa para explicar.");
        return 0;
    }

    if (strcmp(structure->visual_state.layout_mode, "tree_bfs") == 0) {
        suffix = " [Arbol BFS]";
    } else if (strcmp(structure->visual_state.layout_mode, "tree_dfs") == 0) {
        suffix = " [Arbol DFS]";
    } else if (strcmp(structure->visual_state.layout_mode, "tree_prim") == 0) {
        suffix = " [Arbol Prim]";
    } else if (strcmp(structure->visual_state.layout_mode, "tree_kruskal") == 0) {
        suffix = " [Arbol Kruskal]";
    }

    snprintf(line, sizeof(line), "Teoria contextual: %s%s", ss_variant_descriptor(structure->variant)->display_name, suffix);
    ss_theory_append_line(buffer, capacity, line);
    ss_theory_append_line(buffer, capacity, "========================================");

    ss_theory_append_section(buffer, capacity, "Concepto estructural");
    ss_theory_append_structure_summary(structure, buffer, capacity);

    ss_theory_append_section(buffer, capacity, "Familia y variantes");
    ss_theory_append_variants_summary(structure, buffer, capacity);

    ss_theory_append_section(buffer, capacity, "Recorridos y algoritmos");
    ss_theory_append_available_analyses(structure, kind, buffer, capacity);

    ss_error_clear(error);
    return 1;
}
