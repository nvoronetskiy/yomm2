// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <yorel/yomm2.hpp>

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

using namespace yorel::yomm2;
using namespace yorel::yomm2::detail;
using namespace boost::mp11;

// clang-format off

namespace YOMM2_GENSYM {

struct base {
    virtual ~base() {}
};

struct a : base {};
struct b : base {};
struct c : base {};
struct d : base {};
struct e : base {};
struct f : base {};

static_assert(
    std::is_same_v<
        mp_filter<
            is_virtual,
            types< virtual_<a&>, b, virtual_<c&> >
        >,
        types< virtual_<a&>, virtual_<c&> >
    >);

static_assert(
    std::is_same_v<
        remove_virtual<virtual_<a&>>,
        a&
    >);

static_assert(
    std::is_same_v<
        polymorphic_type<default_policy, a&>,
        a
    >);

static_assert(
    std::is_same_v<
        mp_transform<
            remove_virtual,
            types< virtual_<a&>, virtual_<c&> >
        >,
        types<a&, c&>
    >);

static_assert(
    std::is_same_v<
        mp_transform_q<
            mp_bind_front<polymorphic_type, default_policy>,
            mp_transform<
                remove_virtual,
                types< virtual_<a&>, virtual_<c&> >
            >
        >,
        types<a, c>
    >);

static_assert(
    std::is_same_v<
        mp_transform_q<
            mp_bind_front<polymorphic_type, default_policy>,
            mp_transform<
                remove_virtual,
                mp_filter<
                    is_virtual,
                    types< virtual_<a&>, b, virtual_<c&> >
                >
            >
        >,
        types<a, c>
    >);

static_assert(
    std::is_same_v<
        polymorphic_types<types<virtual_<a&>, b, virtual_<c&>>>,
        types<a&, c&>>);

static_assert(
    std::is_same_v<
        polymorphic_types<types<
            virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>>,
        types<std::shared_ptr<a>, std::shared_ptr<c>>>);

static_assert(
    std::is_same_v<
        spec_polymorphic_types<
            default_policy,
            types<virtual_<a&>, b, virtual_<c&>>,
            types<d&, e, f&>>,
        types<d, f>>);

static_assert(
    std::is_same_v<
        polymorphic_type<default_policy, std::shared_ptr<a>>,
    a>);

static_assert(
    std::is_same_v<
        spec_polymorphic_types<
            default_policy,
            types<
                virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>,
            types<std::shared_ptr<d>, e, std::shared_ptr<f>>>,
        types<d, f>>);

BOOST_AUTO_TEST_CASE(test_type_id_list) {
    type_id expected[] = {type_id(&typeid(a)), type_id(&typeid(b))};
    auto iter = type_id_list<default_policy, types<a&, b&>>::begin;
    auto last = type_id_list<default_policy, types<a&, b&>>::end;
    BOOST_TEST_REQUIRE(last - iter == 2);
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(a)));
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(b)));
}

} // namespace YOMM2_GENSYM

namespace casts {

struct Animal {
    virtual ~Animal() {}
    int a{1};
};

struct Mammal : virtual Animal {
    int m{2};
};

struct Carnivore : virtual Animal {
    int c{3};
};

struct Dog : Mammal, Carnivore {
    int d{4};
};

const void* mammal_this(const Mammal& obj) {
    return &obj;
}

const void* carnivore_this(const Carnivore& obj) {
    return &obj;
}

const void* dog_this(const Dog& obj) {
    return &obj;
}

BOOST_AUTO_TEST_CASE(casts) {
    Dog dog;
    const Animal& animal = dog;
    const Mammal& mammal = dog;
    const Carnivore& carnivore = dog;

    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Mammal&>(animal).m)
        == &dog.m);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Carnivore&>(animal).c)
        == &dog.c);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Mammal&>(animal).m)
        == &dog.m);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Animal&>::cast<const Dog&>(animal).d)
        == &dog.d);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Mammal&>::cast<const Dog&>(mammal).d)
        == &dog.d);
    BOOST_TEST(
        (&virtual_traits<default_policy, const Carnivore&>::cast<const Dog&>(carnivore).c)
        == &dog.c);

    using voidp = const void*;
    using virtual_animal_t = polymorphic_type<default_policy, const Animal&>;
    static_assert(std::is_same_v<virtual_animal_t, Animal>, "animal");
    using virtual_mammal_t = polymorphic_type<default_policy, const Mammal&>;
    static_assert(std::is_same_v<virtual_mammal_t, Mammal>, "mammal");

    voidp base_address;

    base_address = thunk<
        default_policy,
        voidp(virtual_<const Animal&>), mammal_this,
        types<const Mammal&>>::fn(animal);
    BOOST_TEST(base_address == &mammal);

    base_address = thunk<
        default_policy,
        voidp(virtual_<const Animal&>), carnivore_this,
        types<const Carnivore&>>::fn(animal);
    BOOST_TEST(base_address == &carnivore);

    base_address = thunk<
        default_policy,
        voidp(virtual_<const Animal&>), mammal_this,
        types<const Dog&>>::fn(animal);
    BOOST_TEST(base_address == &dog);
}

} // namespace casts

namespace test_use_classes {

struct Animal {};
struct Dog : public Animal {};
struct Bulldog : public Dog {};
struct Cat : public Animal {};
struct Dolphin : public Animal {};

static_assert(
    std::is_same_v<
        inheritance_map<Animal, Dog, Bulldog, Cat, Dolphin>,
        types<
            types<Animal, Animal>,
            types<Dog, Animal, Dog>,
            types<Bulldog, Animal, Dog, Bulldog>,
            types<Cat, Animal, Cat>,
            types<Dolphin, Animal, Dolphin>
        >
>);

static_assert(
    std::is_same_v<
        use_classes<Animal, Dog, Bulldog, Cat, Dolphin>,
        std::tuple<
            class_declaration_aux<default_policy, types<Animal, Animal>>,
            class_declaration_aux<default_policy, types<Dog, Animal, Dog>>,
            class_declaration_aux<default_policy, types<Bulldog, Animal, Dog, Bulldog>>,
            class_declaration_aux<default_policy, types<Cat, Animal, Cat>>,
            class_declaration_aux<default_policy, types<Dolphin, Animal, Dolphin>>
        >
>);

static_assert(
    std::is_same_v<
        use_classes_macro<Animal, default_policy>,
        std::tuple<
            class_declaration_aux<default_policy, types<Animal, Animal>>
        >
>);

struct my_policy : policy::abstract_policy {};

static_assert(
    std::is_same_v<
        use_classes_macro<Animal, my_policy, default_policy>,
        std::tuple<
            class_declaration_aux<my_policy, types<Animal, Animal>>
        >
>);

static_assert(
    std::is_same_v<
        use_classes_macro<Animal, my_policy>,
        std::tuple<
            class_declaration_aux<my_policy, types<Animal, Animal>>
        >
>);


} // namespace test_use_classes

namespace facets {

using namespace policy;

struct key1;
struct key2;
struct alt_rtti {};

static_assert(std::is_same_v<
    rebind_facet<key2, basic_domain<key1>>::type,
    basic_domain<key2>
>);

// yorel::yomm2::policy::basic_policy<facets::key2, yorel::yomm2::policy::std_rtti>,
// yorel::yomm2::policy::basic_policy<yorel::yomm2::policy::basic_domain<facets::key2>, yorel::yomm2::policy::std_rtti>

struct policy1 : basic_policy<policy1, std_rtti> {};
struct policy2 : policy1::rebind<policy2> {};
struct policy3 : policy1::rebind<policy3>::replace<std_rtti, alt_rtti> {};

static_assert(std::is_same_v<
    policy2::facets,
    types<std_rtti>
>);

static_assert(std::is_same_v<
    policy3::facets,
    types<alt_rtti>
>);

// static_assert(std::is_same_v<
//     basic_policy<basic_domain<key1>, std_rtti>::replace<std_rtti, alt_rtti>,
//     basic_policy<basic_domain<key1>, alt_rtti>
// >);

}

void f(char, int) {}

static_assert(std::is_same_v<
    parameter_type_list_t<decltype(f)>,
    types<char, int>
>);

// -----------------------------------------------------------------------------
// static_slots

namespace test_static_slots {
struct Animal;
}

namespace yorel {
namespace yomm2 {
namespace detail {

template<>
struct static_offsets<
    method<
        void,
        void (
            virtual_<test_static_slots::Animal&>,
            virtual_<test_static_slots::Animal&>)>
> {
    static constexpr std::size_t slots[] = { 0, 1 };
};

}
}
}

namespace test_static_slots {

struct Animal {
    virtual ~Animal() {}
};

using kick = method<void, void (virtual_<Animal&>)>;
static_assert(!has_static_offsets<kick>::value);

using meet = method<void, void (virtual_<Animal&>, virtual_<Animal&>)>;
static_assert(has_static_offsets<meet>::value);

}

namespace test_report {

struct report {};

struct facet1 {
    struct report {};
};

struct facet2 {
};

struct facet3 {
    struct report {};
};

static_assert(
    std::is_base_of_v<
        report, typename aggregate_reports<
            types<report>, types<facet1, facet2, facet3>>::type>);
static_assert(
    std::is_base_of_v<
        facet1::report, typename aggregate_reports<
            types<report>, types<facet1, facet2, facet3>>::type>);
static_assert(
    std::is_base_of_v<
        facet3::report, typename aggregate_reports<
            types<report>, types<facet1, facet2, facet3>>::type>);

}
