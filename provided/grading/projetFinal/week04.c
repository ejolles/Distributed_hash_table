/**
 * @file week04.c
 * @brief test code for hashtables
 */

#include <check.h>

#include "tests.h"
#include "hashtable.h"

START_TEST(delete_works_correctly_on_linked_list)
{
    Htable_t table = construct_Htable(1);

    add_Htable_value(table, "a", "b");
    add_Htable_value(table, "c", "d");

    delete_Htable_and_content(&table);
}
END_TEST

START_TEST(add_value_does_retrieve_same_value)
{
    Htable_t table = construct_Htable(HTABLE_SIZE);

    pps_key_t cle = "c";
    pps_value_t valeur_mise = "42";
    error_code err = add_Htable_value(table, cle, valeur_mise);

    const pps_value_t valeur_lue = get_Htable_value(table, cle);
    ck_assert_str_eq(valeur_mise, valeur_lue);
    ck_assert_err_none(err);
}
END_TEST

START_TEST(add_value_returns_ERR_NONE)
{
    Htable_t table = construct_Htable(HTABLE_SIZE);
    pps_key_t cle = "c";
    pps_value_t valeur_mise = "42";
    error_code err = add_Htable_value(table, cle, valeur_mise);

    ck_assert_err_none(err);
}
END_TEST

START_TEST(add_value_returns_ERR_BAD_PARAMETER)
{

    pps_key_t cle = "c";
    pps_value_t valeur_mise = "42";
    Htable_t table = {NULL, 0};
    error_code err = add_Htable_value(table, cle, valeur_mise);

    ck_assert_bad_param(err);
}
END_TEST



Suite *hashtable_suite()
{

    Suite *s = suite_create("hashtable.h");

    TCase *tc_ht = tcase_create("hashtable");
    suite_add_tcase(s, tc_ht);

    tcase_add_test(tc_ht, delete_works_correctly_on_linked_list);
    tcase_add_test(tc_ht, add_value_does_retrieve_same_value);
    tcase_add_test(tc_ht,add_value_returns_ERR_NONE );
    tcase_add_test(tc_ht,add_value_returns_ERR_BAD_PARAMETER );
    return s;
}

TEST_SUITE(hashtable_suite)
