# MyDroneProject

Begleit-Repo zur YouTube-Tutorial-Serie "Blueprint zu C++": ein spielbarer, physikalisch
simulierter Quadcopter in Unreal Engine 5.8, komplett in C++ gebaut. Der Git-Verlauf ist
absichtlich in Video-Schritten aufgebaut - jeder Tag `video-01-end`, `video-02-end`, ...
markiert den Code-Stand am Ende des jeweiligen Videos.

## Den Faden verloren? So holst du auf

Falls du im Video nicht mitgekommen bist oder direkt bei einer späteren Folge einsteigen willst,
und du hast dein eigenes `MyDroneProject` schon selbst mit dem UE-Wizard erstellt (wie in Video 1
gezeigt): dein Ordner ist dann nicht leer, `git clone` kann also nicht direkt hineinklonen. Bau dir
stattdessen den gewünschten Stand so auf:

```bash
cd C:\UEProjects\UE5.8
ren MyDroneProject MyDroneProject_eigenerVersuch
git clone https://github.com/volkmerthoms/MyDroneProject.git
cd MyDroneProject
git checkout video-01-end
```

Dein eigener Versuch bleibt dabei als `MyDroneProject_eigenerVersuch` erhalten (umbenannt, nicht
gelöscht) - falls du später nochmal reinschauen willst, was du selbst gebaut hattest.

Für ein späteres Video statt `video-01-end` einfach den passenden Tag auschecken, z.B.:

```bash
git checkout video-04-end
```

## Video-Checkpoints

| Tag             | Inhalt                                         |
|-----------------|--------------------------------------------------|
| `video-01-end`  | Projekt-Setup, minimales kompilierendes Pawn-Skelett |
| `video-02-end`  | Pawn-Komponenten-Hierarchie                     |
| `video-03-end`  | Enhanced Input komplett in C++                  |
| `video-04-end`  | Physik-Flugmodell (AddForce/AddTorqueInDegrees) |
| `video-05-end`  | State Machine, Crash-Erkennung & Battery        |
| `video-06-end`  | HUD in C++ ohne UMG                             |
| `video-07-end`  | Debugging-Folge                                 |

*(Tags werden fortlaufend ergänzt, sobald das jeweilige Video fertig ist.)*
