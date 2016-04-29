
MC723 - Laboratório de Projetos de Sistemas Computacionais
====
######Campinas, 29 de abril de 2016
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

Roteiro:
----
#####Benchmarks escolhidos para realizar testes do processador:
* Jpeg coder (small)
* Rijndael coder (small)
* GSM coder (large)
* Dijkstra (large)
 
#####Hazard de dados e de controle
* Load e Store ou quaisquers duas instruções que compartilham dados de um mesmo registrador
* Jump e branch

Para contar o número de chamadas a essas funções, utilizaremos a flag "-s" do simulador

#####Eventos a serem avaliados:
* Miss L1
* Miss L2
* Número de Hazards de controle
* Numero de branchs realizados
* Número de branchs previstos
* Ciclos 
* Tempo

#####Configurações:
* Branch
  * Always taken
  * Branch history table
* Configurações de cache
  * Serão utilizados os melhores casos do exercício anterior pra cada benchmark (4 diferentes)
* Pipeline
  * 5 estágios
  * 7 estágios
  * 13 estágios
  

#####Cronograma:
* 1ª semana:
  * Aprender a configurar e executar as ferramentas (ex.: ArchC e MIPS)
* 2ª semana:
  * Realizar experimentos e medições
* 3ª semana:
  * Escrever relatórios e slides e preparar para apresentação




Referências
----
1. http://www.ic.unicamp.br/~lucas/teaching/mc723/2016-1/p2.html




