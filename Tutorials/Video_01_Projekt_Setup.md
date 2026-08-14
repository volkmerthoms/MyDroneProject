# Video 1: Projekt-Setup & die Blueprint-Brücke

**Zielgruppe:** UE-Anwender, die Blueprint kennen und jetzt C++ lernen
**Ziel-Länge:** 15–20 Min
**Projekt:** MyDroneProject (UE 5.8, eigenes C++-Modul `MyDroneProject`)

## Lernziele dieser Folge
- Verstehen, warum `.uproject` und `.Build.cs` die Basis jedes C++-Moduls sind
- `UCLASS`, `GENERATED_BODY`, `UPROPERTY`, `UFUNCTION` als Brücke zwischen C++ und Blueprint begreifen
- Wissen, was beim Kompilieren passiert (UBT + UHT), und warum das anders tickt als der BP-Compiler

---

## 00:00 – Cold Open / Hook

**[Bildschirm: PIE läuft, Drohne fliegt, HUD zeigt Throttle/Battery/Status]**

**Sprechtext:**
"Das hier ist eine Drohne, die zu 100% in C++ fliegt — kein einziger Movement-Blueprint-Node. Component-Hierarchie, Input-Handling, Physik, sogar das HUD: alles Code. Wenn du bisher nur Blueprint gemacht hast, siehst du in dieser Serie Schritt für Schritt, wie dasselbe Denken, das du aus BP kennst, in C++ aussieht — und vor allem: wo die beiden sich treffen. Das ist nämlich der Punkt, an dem die meisten BP-Leute hängen bleiben: sie denken, C++ und Blueprint sind zwei getrennte Welten. Sind sie nicht."

---

## 01:00 – Was du am Ende dieser Serie kannst

**Sprechtext:**
"Am Ende dieser Serie hast du eine eigene, voll funktionsfähige Drohnen-Pawn-Klasse in C++, inklusive Enhanced Input, Physik-Flugmodell und HUD — und du verstehst, warum jede einzelne Zeile so und nicht anders geschrieben ist. Wichtig: Wir fangen heute bewusst nur mit dem C++-Teil an. Alles unter Game/Content — Meshes, Materials, die eigentliche BP_DroneVehicle, die das hier hübsch macht — kommt später dran. Heute geht's um das Fundament."

**[Bildschirm: Ordnerstruktur des Projekts zeigen — Content Browser kurz aufklappen, dann wieder wegklicken]**

---

## 02:00 – Die .uproject-Datei

**[Bildschirm: `MyDroneProject.uproject` in VS Code/Editor öffnen]**

**Sprechtext:**
"Fangen wir ganz unten an — bei der Datei, die überhaupt erst definiert, dass es ein C++-Projekt ist: die `.uproject`."

**Code zeigen:**
```json
{
	"FileVersion": 3,
	"EngineAssociation": "5.8",
	"Modules": [
		{
			"Name": "MyDroneProject",
			"Type": "Runtime",
			"LoadingPhase": "Default"
		}
	],
	"Plugins": [ ... ]
}
```

**Sprechtext:**
"Der Teil, der bei einem reinen Blueprint-Projekt fehlt, ist der `Modules`-Block. Der sagt der Engine: 'Es gibt ein natives Code-Modul namens MyDroneProject, lade es beim Start.' Ohne diesen Eintrag würde Unreal dein C++ gar nicht erst einbinden — dann bräuchtest du gar keine kompilierte DLL, das Projekt würde einfach als reines BP-Projekt starten.

Der `Plugins`-Block daneben kennst du schon aus dem Plugin-Browser im Editor — das ist einfach die textuelle Repräsentation von dem, was du dort an- und abhakst."

---

## 04:00 – MyDroneProject.Build.cs: die Abhängigkeiten

**[Bildschirm: `Source/MyDroneProject/MyDroneProject.Build.cs` öffnen]**

**Sprechtext:**
"Die zweite Datei, die es in Blueprint-only-Projekten nicht gibt: die `.Build.cs`. Stell sie dir vor wie die Blueprint-Klasseneinstellungen — nur dass du hier nicht Interfaces oder Parent-Class anklickst, sondern *Module*, von denen dein Code Funktionen benutzen darf."

**Code zeigen:**
```csharp
public class MyDroneProject : ModuleRules
{
	public MyDroneProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput"
		});
	}
}
```

**Sprechtext:**
"`Core`, `CoreUObject`, `Engine` — die drei hast du praktisch immer dabei, das ist der Unterbau von allem: `UObject`, `AActor`, `APawn`, all das kommt von hier. `EnhancedInput` brauchen wir, weil unsere Drohne ihr komplettes Input-System in C++ aufbaut, ohne ein einziges Input-Action-Asset — dazu mehr in Folge 3.

Merk dir die Faustregel: Wenn du in C++ eine Klasse aus einem UE-Modul benutzen willst und der Compiler sagt 'unresolved external symbol' oder 'file not found' — dann fehlt hier fast immer der Eintrag."

**[Kurzer Cutaway: einen absichtlichen Fehler zeigen — Modul rausnehmen, Build starten, den Fehler im Output sehen, wieder reinmachen]**

---

## 06:30 – Der Elefant im Raum: UCLASS, GENERATED_BODY & Co.

**[Bildschirm: `DroneVehiclePawn.h`, obere Hälfte]**

**Sprechtext:**
"Und jetzt zum eigentlichen Kern dieser Folge — dem Teil, der dir als BP-Umsteiger am fremdesten vorkommen wird, aber eigentlich der freundlichste Teil von allen ist."

**Code zeigen:**
```cpp
UCLASS()
class MYDRONEPROJECT_API ADroneVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	ADroneVehiclePawn();
	...
```

**Sprechtext:**
"`UCLASS()` und `GENERATED_BODY()` sind kein normales C++. Das ist Unreals *Reflection-System*. Wenn du das kompilierst, läuft vor dem eigentlichen C++-Compiler ein Tool namens Unreal Header Tool — kurz UHT — über deine Header-Dateien und generiert zusätzlichen Code drumherum. Der Grund, warum das existiert: Blueprint muss zur Laufzeit wissen, welche Properties und Funktionen deine C++-Klasse hat, um sie im Details-Panel anzuzeigen oder im Graph aufrufbar zu machen. Ohne `UCLASS`/`GENERATED_BODY` ist deine Klasse für Blueprint komplett unsichtbar — sie existiert nur in C++."

---

## 08:30 – UPROPERTY: die BP-Variable in C++

**Code zeigen:**
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Flight Model")
float MaxThrustPerRotor = 550.f;

UPROPERTY(BlueprintReadOnly, Category = "Drone|State")
float CurrentThrottle01 = 0.f;
```

**Sprechtext:**
"Erinnerst du dich an die Häkchen, die du bei einer BP-Variable im Details-Panel setzt — 'Instance Editable', 'Expose on Spawn', 'BlueprintReadOnly'? Genau das sind diese Specifier in den Klammern, nur eine Zeile früher, direkt am C++-Member.

`EditAnywhere` heißt: im Editor pro Instanz und in der Blueprint-Child-Class veränderbar — wie 'Instance Editable' in BP. `BlueprintReadWrite` heißt: aus dem Blueprint-Graph lesbar *und* schreibbar. `BlueprintReadOnly` — nur lesbar, du siehst den Wert im Graph, kannst ihn aber nicht von dort aus setzen, weil C++ die alleinige Kontrolle behalten soll. `Category` ist exakt das, was du im Details-Panel als Gruppierung siehst.

Ohne `UPROPERTY` ist die Variable für BP so unsichtbar wie eine ganz normale, private C++-Variable — sie existiert, aber niemand außerhalb von C++ kommt dran."

---

## 10:30 – UFUNCTION: die BP-Node in C++

**Code zeigen:**
```cpp
UFUNCTION(BlueprintCallable, Category = "Drone|Input")
void SetThrottleInput(float Value);

UFUNCTION(BlueprintPure, Category = "Drone|Telemetry")
float GetHoverThrottleEstimate() const;

UFUNCTION(BlueprintImplementableEvent, Category = "Drone|Events")
void OnDroneCrashed(float ImpactSpeed);
```

**Sprechtext:**
"Gleiches Prinzip bei Funktionen. `BlueprintCallable` — die Funktion taucht im BP-Graph als aufrufbarer Node auf, mit Execution-Pin, so wie jeder normale Funktions-Node. `BlueprintPure` — kennst du als die blauen Nodes ohne Execution-Pin, reine Werte-Berechnung ohne Seiteneffekt.

Und dann `BlueprintImplementableEvent` — das ist die Umkehrung von allem bisherigen. Die Funktion hat in C++ *keinen* Body, keine geschweiften Klammern, nur die Deklaration mit Semikolon. Die Implementierung passiert komplett in einer Blueprint-Child-Class, als Event-Node, den du im Graph siehst und mit deinen eigenen Nodes befüllst. Das ist der offizielle 'Hook' zurück nach Blueprint — C++ sagt 'hier ist ein Drohnen-Crash passiert, mit dieser Aufprall-Geschwindigkeit', und was du optisch/akustisch daraus machst, entscheidest du in BP. Genau dieses Muster ziehen wir in Folge 5 nochmal richtig auf."

---

## 12:30 – Kompilieren: Was passiert wirklich

**[Bildschirm: Terminal, Build.bat-Aufruf, Output mitlaufen lassen]**

**Sprechtext:**
"Letzter Punkt für heute, weil er dir sonst in Folge 2 komisch vorkommt: Kompilieren in C++ ist ein anderer Vorgang als BP-Compile. Wenn du auf 'Compile' im Editor klickst oder extern baust, läuft erst UHT über alle deine Header und generiert die `.generated.h`-Dateien — die siehst du normalerweise nie, die liegen im `Intermediate`-Ordner. Danach kompiliert der eigentliche C++-Compiler alles zusammen. Das dauert beim ersten Mal spürbar länger als ein BP-Compile, aber danach übernimmt *Live Coding* — Ctrl+Alt+F11 im laufenden Editor — die meisten kleinen Änderungen fast so schnell wie BP. Nur bei neuen Modul-Abhängigkeiten oder neuen Klassen brauchst du einen vollen Rebuild."

---

## 15:00 – Von Hand-Tippen zu Git-Checkout: Umstieg auf das Repo

**[Bildschirm: Visual Studio schließen, danach den Unreal-Editor für MyDroneProject schließen, falls noch offen]**

**Sprechtext:**
"Bevor wir weitermachen, schließen wir jetzt kurz das aktuelle MyDroneProject wieder — Visual Studio, und falls noch offen, auch den Unreal-Editor dazu. Der Grund: Ab jetzt steigen wir in das eigentliche Drohnenprojekt ein, und dafür wollen wir nicht jede einzelne Zeile C++ von Hand abtippen — das ist fehleranfällig, und ein einziger Tippfehler in einer Header-Datei kann dich zehn Minuten Fehlersuche kosten, ohne dass du dabei irgendwas Neues lernst.

Stattdessen arbeiten wir ab jetzt mit dem Begleit-Repository auf GitHub. Unter github.com/volkmerthoms/MyDroneProject findest du den kompletten Code dieser Serie: einen master-Branch mit dem aktuellen Gesamtstand, und für jedes einzelne Video einen eigenen Tag — video-01-end, video-02-end und so weiter — der genau den Code-Stand am Ende dieses Videos festhält."

**[Bildschirm: GitHub-Repo-Seite zeigen, Tags-Übersicht aufklappen]**

**Sprechtext:**
"Wichtig, bevor wir loslegen: Die Git-Befehle, die wir gleich nutzen, funktionieren in jedem Terminal, das Zugriff auf git hat — du brauchst kein spezielles Tool dafür. Git Bash, PowerShell, die Windows-Eingabeaufforderung, oder das integrierte Terminal in VS Code oder im Unreal-Editor-Umfeld — nimm einfach das, mit dem du dich am wohlsten fühlst."

**[Bildschirm: Terminal öffnen]**

**Sprechtext:**
"Jetzt zur eigentlichen Umstellung. Du hast dein MyDroneProject ja bereits selbst mit dem UE-Wizard erstellt, so wie eben in diesem Video gezeigt — dein Ordner ist also nicht leer. Das heißt: ein einfaches `git clone` kann da nicht direkt reinklonen, weil Git niemals in einen bereits bestehenden, nicht-leeren Ordner hineinklont. Deshalb gehen wir so vor: Wir wechseln zuerst per `cd` in das Verzeichnis, in dem dein Projektordner liegt — achte hier wirklich darauf, dass du in *dein* Projektverzeichnis wechselst, bei mir ist das `C:\UEProjects\UE5.8` — und benennen den bestehenden Ordner einfach um, statt ihn zu löschen. So bleibt dein eigener Versuch als Backup erhalten, falls du später nochmal reinschauen willst, was du selbst gebaut hattest."

**[Bildschirm: Terminal-Befehle Schritt für Schritt eintippen]**

```
cd C:\UEProjects\UE5.8
ren MyDroneProject MyDroneProject_eigenerVersuch
git clone https://github.com/volkmerthoms/MyDroneProject.git
cd MyDroneProject
git checkout video-01-end
```

**Sprechtext:**
"Nach diesen fünf Zeilen hast du exakt den Stand von Video 1 — geklont, nicht abgetippt, garantiert ohne Tippfehler. Und falls du später bei einem anderen Video einsteigen willst, brauchst du nur noch einen einzigen Befehl, zum Beispiel:

`git checkout video-04-end`

— und schon hast du den Stand von Video 4, ganz ohne die Videos 2 und 3 selbst nachzubauen."

**[Bildschirm: `MyDroneProject.uproject` im geklonten Ordner öffnen, UE Editor startet]**

**Sprechtext:**
"Ab jetzt arbeiten wir aus diesem geklonten Ordner weiter."

---

## 18:30 – Ausblick & Outro

**Sprechtext:**
"Nächstes Mal bauen wir den eigentlichen Pawn zusammen — Components per Code statt per Editor-Klick, und warum `CreateDefaultSubobject` das Gegenstück zum 'Add Component'-Button ist. Wenn du bis hierhin mitgekommen bist: du hast gerade das wichtigste Konzept der ganzen Serie verstanden — `UPROPERTY`/`UFUNCTION` *ist* die Brücke. Alles danach ist nur noch Anwendung davon. Bis nächstes Mal."

**[Outro-Card / Abo-Hinweis]**

---

## Notizen für die Aufnahme
- Bei 04:00 (Build.cs-Fehler-Demo): vorher testen, dass der Fehler klar lesbar im Output erscheint, ggf. Fenster vergrößern
- Bei 12:30: Build-Log ist erfahrungsgemäß sehr voll — vorher einen kurzen Lauf cachen/aufnehmen und im Schnitt beschleunigen, live abwarten killt das Tempo
- Referenzierte Dateien in dieser Folge: `MyDroneProject.uproject`, `Source/MyDroneProject/MyDroneProject.Build.cs`, `Source/MyDroneProject/DroneVehiclePawn.h`
