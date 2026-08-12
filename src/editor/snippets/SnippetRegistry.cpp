#include "editor/snippets/SnippetRegistry.h"
#include <algorithm>

namespace OpenIDE::Editor::Snippets {

SnippetRegistry& SnippetRegistry::instance() {
    static SnippetRegistry s_instance;
    return s_instance;
}

SnippetRegistry::SnippetRegistry() {
    registerDefaultCppSnippets();
    registerUnrealSnippets();
}

void SnippetRegistry::registerSnippet(const SnippetDefinition& snippet) {
    m_snippets.push_back(snippet);
}

std::vector<SnippetDefinition> SnippetRegistry::findMatching(const QString& prefix, const QString& language) const {
    std::vector<SnippetDefinition> results;
    for (const auto& snip : m_snippets) {
        if (snip.language == language || snip.language == "all") {
            if (prefix.isEmpty()) {
                results.push_back(snip);
                continue;
            }

            bool matches = false;
            // 1. Primary trigger prefix match
            if (snip.trigger.startsWith(prefix, Qt::CaseInsensitive)) {
                matches = true;
            }
            // 2. Alias match
            else {
                for (const auto& alias : snip.aliases) {
                    if (alias.startsWith(prefix, Qt::CaseInsensitive)) {
                        matches = true;
                        break;
                    }
                }
            }

            if (matches) {
                results.push_back(snip);
            }
        }
    }
    return results;
}

const SnippetDefinition* SnippetRegistry::findByTrigger(const QString& trigger, const QString& language) const {
    for (const auto& snip : m_snippets) {
        if (snip.language == language || snip.language == "all") {
            if (snip.trigger.compare(trigger, Qt::CaseInsensitive) == 0) {
                return &snip;
            }
            for (const auto& alias : snip.aliases) {
                if (alias.compare(trigger, Qt::CaseInsensitive) == 0) {
                    return &snip;
                }
            }
        }
    }
    return nullptr;
}

std::vector<CompletionItemData> SnippetRegistry::toCompletionItems(const QString& prefix, const QString& language) const {
    std::vector<CompletionItemData> items;
    auto matches = findMatching(prefix, language);

    for (const auto& snip : matches) {
        CompletionItemData item;
        item.label = snip.trigger;
        item.detail = snip.description;
        item.documentation = snip.body;
        item.insertText = snip.body;
        item.kind = 15; // Snippet icon
        item.isSnippet = true;
        item.source = CompletionSource::Snippet;
        items.push_back(item);
    }

    return items;
}

void SnippetRegistry::registerDefaultCppSnippets() {
    // LOOPS & CONTROL FLOW
    registerSnippet({"for", {}, "C++ for loop", "Loops", "for (int ${1:i} = ${2:0}; ${1:i} < ${3:count}; ++${1:i})\n{\n\t$0\n}", "cpp", 200});
    registerSnippet({"for-range", {"forr", "range-for"}, "C++ range-based for loop", "Loops", "for (const auto& ${1:item} : ${2:container})\n{\n\t$0\n}", "cpp", 190});
    registerSnippet({"for-value", {}, "Range for loop by value", "Loops", "for (auto ${1:item} : ${2:container})\n{\n\t$0\n}", "cpp", 180});
    registerSnippet({"for-ref", {}, "Range for loop by reference", "Loops", "for (auto& ${1:item} : ${2:container})\n{\n\t$0\n}", "cpp", 180});
    registerSnippet({"while", {}, "C++ while loop", "Loops", "while (${1:condition})\n{\n\t$0\n}", "cpp", 200});
    registerSnippet({"do-while", {"do"}, "C++ do-while loop", "Loops", "do\n{\n\t$0\n} while (${1:condition});", "cpp", 190});

    // CONDITIONALS
    registerSnippet({"if", {}, "C++ if statement", "Control Flow", "if (${1:condition})\n{\n\t$0\n}", "cpp", 200});
    registerSnippet({"if-else", {"ife"}, "C++ if / else statement", "Control Flow", "if (${1:condition})\n{\n\t$1\n}\nelse\n{\n\t$0\n}", "cpp", 190});
    registerSnippet({"else-if", {"elif"}, "C++ else-if statement", "Control Flow", "else if (${1:condition})\n{\n\t$0\n}", "cpp", 180});
    registerSnippet({"if-not", {}, "C++ if not statement", "Control Flow", "if (!${1:condition})\n{\n\t$0\n}", "cpp", 180});
    registerSnippet({"if-init", {}, "C++17 if with initializer", "Control Flow", "if (auto ${1:value} = ${2:expression}; ${1:value})\n{\n\t$0\n}", "cpp", 180});
    registerSnippet({"switch", {}, "C++ switch statement", "Control Flow", "switch (${1:value})\n{\ncase ${2:0}:\n\t$0\n\tbreak;\n\ndefault:\n\tbreak;\n}", "cpp", 200});
    registerSnippet({"case", {}, "C++ case clause", "Control Flow", "case ${1:value}:\n\t$0\n\tbreak;", "cpp", 190});
    registerSnippet({"default", {}, "C++ default clause", "Control Flow", "default:\n\t$0\n\tbreak;", "cpp", 180});

    // CLASSES & DECLARATIONS
    registerSnippet({"class", {}, "C++ class declaration", "Classes", "class ${1:ClassName}\n{\npublic:\n\t${1:ClassName}();\n\t~${1:ClassName}();\n\nprivate:\n\t$0\n};", "cpp", 200});
    registerSnippet({"struct", {}, "C++ struct declaration", "Structs", "struct ${1:StructName}\n{\n\t$0\n};", "cpp", 200});
    registerSnippet({"enum", {}, "C++ enum class declaration", "Enums", "enum class ${1:EnumName}\n{\n\t${2:Value},\n\t$0\n};", "cpp", 200});
    registerSnippet({"namespace", {}, "C++ namespace block", "Declarations", "namespace ${1:Name}\n{\n\t$0\n}", "cpp", 200});
    registerSnippet({"function", {"func"}, "C++ function definition", "Functions", "${1:void} ${2:FunctionName}(${3:parameters})\n{\n\t$0\n}", "cpp", 200});
    registerSnippet({"function-const", {}, "C++ const member function", "Functions", "${1:void} ${2:FunctionName}(${3:parameters}) const\n{\n\t$0\n}", "cpp", 190});
    registerSnippet({"lambda", {}, "C++ lambda expression", "Lambdas", "[${1:capture}](${2:parameters})\n{\n\t$0\n}", "cpp", 190});
    registerSnippet({"lambda-auto", {}, "C++ auto lambda assignment", "Lambdas", "auto ${1:name} = [${2:&}](auto ${3:value})\n{\n\t$0\n};", "cpp", 180});
    registerSnippet({"ctor", {}, "C++ constructor implementation", "Classes", "${1:ClassName}::${1:ClassName}()\n{\n\t$0\n}", "cpp", 190});
    registerSnippet({"dtor", {}, "C++ destructor implementation", "Classes", "${1:ClassName}::~${1:ClassName}()\n{\n\t$0\n}", "cpp", 190});

    // EXCEPTIONS & ASSERTIONS
    registerSnippet({"try", {}, "C++ try-catch block", "Exceptions", "try\n{\n\t$0\n}\ncatch (const std::exception& ${1:e})\n{\n\t\n}", "cpp", 200});
    registerSnippet({"catch", {}, "C++ catch block", "Exceptions", "catch (const std::exception& ${1:e})\n{\n\t$0\n}", "cpp", 190});
    registerSnippet({"throw", {}, "C++ throw exception", "Exceptions", "throw std::runtime_error(\"${1:message}\");", "cpp", 180});
    registerSnippet({"assert", {}, "C++ assert expression", "Assertions", "assert(${1:condition});", "cpp", 180});
    registerSnippet({"static_assert", {}, "C++ static_assert expression", "Assertions", "static_assert(${1:condition}, \"${2:message}\");", "cpp", 180});

    // SMART POINTERS & MEMORY
    registerSnippet({"unique_ptr", {}, "std::unique_ptr declaration", "Memory", "std::unique_ptr<${1:Type}> ${2:name} = std::make_unique<${1:Type}>(${3:args});", "cpp", 190});
    registerSnippet({"shared_ptr", {}, "std::shared_ptr declaration", "Memory", "std::shared_ptr<${1:Type}> ${2:name} = std::make_shared<${1:Type}>(${3:args});", "cpp", 190});
    registerSnippet({"weak_ptr", {}, "std::weak_ptr declaration", "Memory", "std::weak_ptr<${1:Type}> ${2:name};", "cpp", 180});
    registerSnippet({"make_unique", {}, "std::make_unique call", "Memory", "std::make_unique<${1:Type}>(${2:args})", "cpp", 180});
    registerSnippet({"make_shared", {}, "std::make_shared call", "Memory", "std::make_shared<${1:Type}>(${2:args})", "cpp", 180});

    // STL CONTAINERS & ALGORITHMS
    registerSnippet({"vector", {}, "std::vector container", "STL", "std::vector<${1:Type}> ${2:name};", "cpp", 180});
    registerSnippet({"array", {}, "std::array container", "STL", "std::array<${1:Type}, ${2:Size}> ${3:name};", "cpp", 180});
    registerSnippet({"map", {}, "std::map container", "STL", "std::map<${1:Key}, ${2:Value}> ${3:name};", "cpp", 180});
    registerSnippet({"unordered_map", {}, "std::unordered_map container", "STL", "std::unordered_map<${1:Key}, ${2:Value}> ${3:name};", "cpp", 180});
    registerSnippet({"set", {}, "std::set container", "STL", "std::set<${1:Type}> ${2:name};", "cpp", 180});
    registerSnippet({"unordered_set", {}, "std::unordered_set container", "STL", "std::unordered_set<${1:Type}> ${2:name};", "cpp", 180});
    registerSnippet({"queue", {}, "std::queue container", "STL", "std::queue<${1:Type}> ${2:name};", "cpp", 180});
    registerSnippet({"stack", {}, "std::stack container", "STL", "std::stack<${1:Type}> ${2:name};", "cpp", 180});
    registerSnippet({"find", {}, "std::find algorithm", "STL", "auto ${1:it} = std::find(${2:begin}, ${3:end}, ${4:value});", "cpp", 180});
    registerSnippet({"find-if", {}, "std::find_if algorithm", "STL", "auto ${1:it} = std::find_if(${2:begin}, ${3:end},\n\t[](${4:const auto& item})\n\t{\n\t\treturn ${5:condition};\n\t});", "cpp", 180});
    registerSnippet({"sort", {}, "std::sort algorithm", "STL", "std::sort(${1:begin}, ${2:end});", "cpp", 180});
    registerSnippet({"sort-custom", {}, "std::sort with predicate", "STL", "std::sort(${1:begin}, ${2:end},\n\t[](${3:const auto& a}, ${4:const auto& b})\n\t{\n\t\treturn ${5:a} < ${6:b};\n\t});", "cpp", 180});
    registerSnippet({"transform", {}, "std::transform algorithm", "STL", "std::transform(${1:begin}, ${2:end}, ${3:output},\n\t[](${4:const auto& item})\n\t{\n\t\treturn ${5:expression};\n\t});", "cpp", 180});
    registerSnippet({"foreach", {}, "std::for_each algorithm", "STL", "std::for_each(${1:begin}, ${2:end},\n\t[](${3:const auto& item})\n\t{\n\t\t$0\n\t});", "cpp", 180});

    // CASTS
    registerSnippet({"static_cast", {}, "static_cast expression", "Casts", "static_cast<${1:Type}>(${2:expression})", "cpp", 180});
    registerSnippet({"dynamic_cast", {}, "dynamic_cast expression", "Casts", "dynamic_cast<${1:Type}>(${2:expression})", "cpp", 180});
    registerSnippet({"const_cast", {}, "const_cast expression", "Casts", "const_cast<${1:Type}>(${2:expression})", "cpp", 180});
    registerSnippet({"reinterpret_cast", {}, "reinterpret_cast expression", "Casts", "reinterpret_cast<${1:Type}>(${2:expression})", "cpp", 180});
}

void SnippetRegistry::registerUnrealSnippets() {
    registerSnippet({"uclass", {"class"}, "Unreal UCLASS declaration", "Unreal Classes", "UCLASS()\nclass ${1:CLASS_NAME} : public ${2:UObject}\n{\n\tGENERATED_BODY()\n\npublic:\n\t$0\n};", "cpp", 300});
    registerSnippet({"ustruct", {"struct"}, "Unreal USTRUCT declaration", "Unreal Reflection", "USTRUCT(BlueprintType)\nstruct ${1:STRUCT_NAME}\n{\n\tGENERATED_BODY()\n\npublic:\n\t$0\n};", "cpp", 300});
    registerSnippet({"uenum", {"enum"}, "Unreal UENUM declaration", "Unreal Reflection", "UENUM(BlueprintType)\nenum class ${1:EName} : uint8\n{\n\t${2:Value},\n\t$0\n};", "cpp", 300});
    registerSnippet({"uproperty", {"prop"}, "Unreal UPROPERTY macro", "Unreal Reflection", "UPROPERTY(${1:EditAnywhere, BlueprintReadWrite})\n${2:Type} ${3:PropertyName};", "cpp", 300});
    registerSnippet({"ufunction", {"func"}, "Unreal UFUNCTION macro", "Unreal Reflection", "UFUNCTION(${1:BlueprintCallable})\n${2:void} ${3:FunctionName}();", "cpp", 300});
    registerSnippet({"generated", {}, "Unreal GENERATED_BODY macro", "Unreal Reflection", "GENERATED_BODY()", "cpp", 300});
    registerSnippet({"actor", {"class"}, "Unreal AActor class declaration", "Unreal Classes", "UCLASS()\nclass ${1:CLASS_NAME} : public AActor\n{\n\tGENERATED_BODY()\n\npublic:\n\t${1:CLASS_NAME}();\n\nprotected:\n\tvirtual void BeginPlay() override;\n\npublic:\n\tvirtual void Tick(float DeltaTime) override;\n\nprivate:\n\t$0\n};", "cpp", 290});
    registerSnippet({"character", {}, "Unreal ACharacter class declaration", "Unreal Classes", "UCLASS()\nclass ${1:CLASS_NAME} : public ACharacter\n{\n\tGENERATED_BODY()\n\npublic:\n\t${1:CLASS_NAME}();\n\nprotected:\n\tvirtual void BeginPlay() override;\n\npublic:\n\tvirtual void Tick(float DeltaTime) override;\n\nprivate:\n\t$0\n};", "cpp", 290});
    registerSnippet({"beginplay", {}, "Unreal BeginPlay implementation", "Unreal Gameplay", "void ${1:ClassName}::BeginPlay()\n{\n\tSuper::BeginPlay();\n\t$0\n}", "cpp", 290});
    registerSnippet({"tick", {}, "Unreal Tick implementation", "Unreal Gameplay", "void ${1:ClassName}::Tick(float DeltaTime)\n{\n\tSuper::Tick(DeltaTime);\n\t$0\n}", "cpp", 290});
    registerSnippet({"uconstructor", {}, "Unreal Actor constructor", "Unreal Gameplay", "${1:ClassName}::${1:ClassName}()\n{\n\tPrimaryActorTick.bCanEverTick = true;\n\t$0\n}", "cpp", 290});
    registerSnippet({"component", {}, "Unreal Component property", "Unreal Components", "UPROPERTY(VisibleAnywhere, BlueprintReadOnly)\nTObjectPtr<${1:USceneComponent}> ${2:Component};", "cpp", 290});
    registerSnippet({"meshcomponent", {}, "Unreal StaticMeshComponent property", "Unreal Components", "UPROPERTY(VisibleAnywhere, BlueprintReadOnly)\nTObjectPtr<UStaticMeshComponent> ${1:MeshComponent};", "cpp", 290});
    registerSnippet({"uelog", {}, "Unreal UE_LOG message", "Unreal Logging", "UE_LOG(LogTemp, Log, TEXT(\"${1:Message}\"));", "cpp", 280});
    registerSnippet({"uewarning", {}, "Unreal UE_LOG warning", "Unreal Logging", "UE_LOG(LogTemp, Warning, TEXT(\"${1:Message}\"));", "cpp", 280});
    registerSnippet({"ueerror", {}, "Unreal UE_LOG error", "Unreal Logging", "UE_LOG(LogTemp, Error, TEXT(\"${1:Message}\"));", "cpp", 280});
    registerSnippet({"cast", {}, "Unreal Cast expression", "Unreal Gameplay", "Cast<${1:AActor}>(${2:Object})", "cpp", 280});
    registerSnippet({"getworld", {}, "Unreal GetWorld call", "Unreal Gameplay", "GetWorld()", "cpp", 280});
    registerSnippet({"spawnactor", {}, "Unreal SpawnActor call", "Unreal Gameplay", "GetWorld()->SpawnActor<${1:AActor}>(${2:ActorClass}, ${3:Transform});", "cpp", 280});
    registerSnippet({"destroyactor", {}, "Unreal Actor Destroy call", "Unreal Gameplay", "${1:Actor}->Destroy();", "cpp", 280});
    registerSnippet({"timer", {}, "Unreal SetTimer call", "Unreal Gameplay", "GetWorld()->GetTimerManager().SetTimer(${1:TimerHandle}, this, &${2:ClassName}::${3:FunctionName}, ${4:Delay}, ${5:true});", "cpp", 280});
    registerSnippet({"serverrpc", {}, "Unreal Server RPC declaration", "Unreal Networking", "UFUNCTION(Server, Reliable)\nvoid ${1:FunctionName}();", "cpp", 280});
    registerSnippet({"clientrpc", {}, "Unreal Client RPC declaration", "Unreal Networking", "UFUNCTION(Client, Reliable)\nvoid ${1:FunctionName}();", "cpp", 280});
    registerSnippet({"multicast", {}, "Unreal NetMulticast RPC declaration", "Unreal Networking", "UFUNCTION(NetMulticast, Reliable)\nvoid ${1:FunctionName}();", "cpp", 280});
    registerSnippet({"delegate", {}, "Unreal DECLARE_DELEGATE", "Unreal Delegates", "DECLARE_DELEGATE(${1:FDelegate});", "cpp", 280});
    registerSnippet({"multicastdelegate", {}, "Unreal DECLARE_MULTICAST_DELEGATE", "Unreal Delegates", "DECLARE_MULTICAST_DELEGATE(${1:FDelegate});", "cpp", 280});
    registerSnippet({"dynamicdelegate", {}, "Unreal DECLARE_DYNAMIC_DELEGATE", "Unreal Delegates", "DECLARE_DYNAMIC_DELEGATE(${1:FDelegate});", "cpp", 280});
    registerSnippet({"uinterface", {}, "Unreal UINTERFACE declaration", "Unreal Classes", "UINTERFACE(BlueprintType)\nclass ${1:INTERFACE_NAME} : public UInterface\n{\n\tGENERATED_BODY()\n};", "cpp", 280});
}

} // namespace OpenIDE::Editor::Snippets
