#pragma once
#include "Registry.h"
#include "Entity.h"
#include <tuple>
#include <functional>
#include "Components/ComponentIndex.h"

namespace Mupfel {

    template<typename... Components>
    class View {
    public:
        using component_types = std::tuple<Components...>;
        using BaseComponent = std::tuple_element_t<0, component_types>;

        explicit View(Registry& in_reg)
            : reg(in_reg)
        {
            // Erzeuge die benoetigte Components-Signatur
            required_signature = ((1ull << ComponentIndex::Index<Components>()) | ...);
        }

        struct Iterator {
            Registry& registry;
            size_t index;
            Entity::Signature required_signature;

            Iterator(Registry& reg,
                uint64_t req,
                size_t idx)
                : registry(reg), index(idx), required_signature(req)
            {
                SkipInvalid();
            }

            void SkipInvalid() {
                auto& comp_array = registry.GetComponentArray<BaseComponent>();
                while (index < comp_array.Size()) {
                    Entity e{ comp_array.dense[index] };
                    Entity::Signature sig = registry.GetSignature(e.Index());

                    if ((sig & required_signature) == required_signature)
                        break;

                    ++index;
                }
            }

            bool operator!=(const Iterator& o) const { return index != o.index; }

            void operator++() {
                ++index;
                SkipInvalid();
            }

            auto operator*() {
                auto& comp_array = registry.GetComponentArray<BaseComponent>();
                Entity e{ comp_array.dense[index] };

                // Fold-Expression: tuple of references erzeugen
                auto tuple_of_refs =
                    std::tuple<Components&...>(registry.GetComponent<Components>(e)...);

                return std::tuple_cat(std::make_tuple(e), tuple_of_refs);
            }
        };

    public:
        Iterator begin() {
            return Iterator(reg, required_signature, 0);
        }

        Iterator end() {
            auto& array = reg.GetComponentArray<BaseComponent>();
            return Iterator(reg, required_signature, array.Size());
        }

    private:
        Registry& reg;
        uint64_t required_signature = 0;
    };

}