
MC723 - Laboratório de Projetos de Sistemas Computacionais
====
######Campinas, 06 de maio de 2016
####Professor: Lucas Wanner
####RA: &nbsp; &nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp; Alunos:
####&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;136242 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp; João Guilherme Daros Fidélis
####&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;139546 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp; João Pedro Ramos Lopes
####&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;145539 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp; Bruno Takeshi Hori
####&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;146009 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; &nbsp; &nbsp;&nbsp; &nbsp; &nbsp; Felipe de Oliveira Emos

Projeto 2 - Desempenho do Processador
====

Objetivos:
----
Medir desempenho de um processador em certos pontos:
* Tamanho do pipeline: 5, 7 e 13 estágios
* Processador escalar vs superescalar
* Hazard de dados e controle
* Branch predictor (2 configurações distintas)
* Cache (4 configurações distintas)

Sumário
----
* [**Roteiro**](ROTEIRO.md)
* [**Tratamento de Hazards**](HAZARD_TREATMENT.md)
 
Traces
----

Hazards de dados
----
Após descobrir os tipos de hazards de dados existentes (veja Sumário), foram feitas formas diferentes em relação à escalar e superescalar. Porém, a ideia básica foi a mesma: criar um vetor de structs com as informações da instrução em cada estágio.

__Escalar:__ Como o único hazard de dados que não pode ser resolvido por forwarding é RAW, basta conferir se a instrução anterior é do tipo _load_ e se o registrador é um dos registradores da instrução atual.

__Superescalar:__ Neste caso, foi utilizado um pipeline 2-way.

Como todos os tipos de hazards de dados (RAW, WAR e WAW) podem acontecer, 



Hazards de Controle
----
Foram feitos três tipos de configurações em relação ao branch prediction: sem branch prediction, branch prediction always taken e branch prediction 2-bit prediction.

__Sem Branch prediction:__ O código apenas soma as bolhas que ocorrerão.

__Branch Prediction Always Taken:__ Neste caso, só é contabilizada as bolhas caso o branch não ocorrer. No código, existe uma variável 

Tabelas
----

| Eventos | Config1 | Config2 | Config3 |  |  |  |
| --- | --- | --- | --- | --- | --- | --- |
| Miss L1 |  |  |  |  |  |  |
| Miss L2 |  |  |  |  |  |  |
| Hazards |  |  |  |  |  |  |
| Branch Realizados |  |  |  |  |  |  |
| Branch Previstos |  |  |  |  |  |  |
| Ciclos |  |  |  |  |  |  |
| Tempo |  |  |  |  |  |  |


Referências
----
1. http://www.ic.unicamp.br/~lucas/teaching/mc723/2016-1/p2.html
