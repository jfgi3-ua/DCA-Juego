# Práctica 0 - Diseño Colaborativo de Aplicaciones
> - **Esther Peral Soler**
> - **Hugo Redondo Valdés**
> - **Alejandro Montoya Aracil**
> - **Juan Fernando Ganim Iborra**

# Compilación
~~~
g++ -std=c++17 -o game src/*.cpp -I src -I vendor/include -L vendor/lib -lraylib -lGL -lm -lpthread -lrt -lX11
~~~