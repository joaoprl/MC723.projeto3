Roteiro:
----
#####Benchmarks escolhidos para realizar testes do processador:
* Jpeg coder (small)
* Rijndael coder (small)
* GSM coder (large)
* Dijkstra (large)
 
#####Hazards
* Hazard de dados: Surge quando uma instrução depende de uma anterior que ainda está no pipeline. Pode ser corrigido com forwarding.
* Hazard de controle: Ocorre quando o instruction fetch busca por uma instrução que nunca será executada (por causa de branches). Pode ser minimizado com branch predictions.

Para avaliar os hazards modificaremos o código do simulador, pois é necessário manter um histórico de eventos para computar dependências de instruções.

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
  * Sem branch prediction
* Configurações de cache
  1. Configuração encontrada no exercício 2 por um dos integrantes.
  2. Baseado no processador AMD FX-8350
  3. Baseado no processador AMD Athlon II P340

|Config | L1 isize | L1 ibsize | L1 dsize | L1 dbsize | L2 usize | L2 ubsize |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 32k | 64 | 64k | 64 | 256k | 16 |
| 2 | 128k | 64 | 128k | 64 | 1024k | 64 |
| 3 | 256k | 64 | 128k | 64 | 8192k | 64 |
 

* Pipeline
  * __5 estágios__ (configuração escolhida)
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
2. https://github.com/jpedrorl/MC723/tree/master/exercicio2
3. http://www.notebookcheck.net/AMD-FX-Series-Desktop-FX-8350-Notebook-Processor.91968.0.html
4. http://www.notebookcheck.net/AMD-Athlon-II-P340-Notebook-Processor.37883.0.html
