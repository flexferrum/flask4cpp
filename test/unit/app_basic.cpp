//
// Created by flexf on 12.03.2020.
//

#include <flask4cpp/app.h>
#include <flask4cpp/app_builder.h>
#include <gtest/gtest.h>

TEST(AppBasic, DefaultCreate_Successfull)
{
    flask4cpp::AppBuilder builder;

    builder.ConfigureDefault();
    auto app = builder.CreateApp();
    EXPECT_TRUE(app.has_value());
}

TEST(AppBasic, SimpleCreate_Successfull)
{
    flask4cpp::AppBuilder builder;

    builder.ListenOn("default", "127.0.0.1", 80);
    auto app = builder.CreateApp();
    EXPECT_TRUE(app.has_value());
}

TEST(AppBasic, Create_Failed)
{
    flask4cpp::AppBuilder builder;

    auto app = builder.CreateApp();
    EXPECT_FALSE(app.has_value());
    EXPECT_EQ(flask4cpp::AppBuilder::BuildErrors::NotInitialized, app.error());
}

TEST(AppBasic, DoubleEndpointsCreate_Failed)
{
    flask4cpp::AppBuilder builder;

    builder.ListenOn("default", "127.0.0.1", 80);
    builder.ListenOn("non_default", "127.0.0.1", 80);
    auto app = builder.CreateApp();
    EXPECT_EQ(flask4cpp::AppBuilder::BuildErrors::ConfigurationError, app.error());
}
