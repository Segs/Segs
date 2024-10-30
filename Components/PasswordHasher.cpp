/*
 * SEGS - Super Entity Game Server
 * http://www.segs.dev/
 * Copyright (c) 2006 - 2019 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup Components
 * @{
 */

#include "PasswordHasher.h"

#include "Utils/string_utils.h"

#include <chrono>
#include <cmath>

uint32_t msecsSinceStartOfDay()
{
    using namespace std::chrono;

    // Get current time point
    auto now = system_clock::now();

    // Convert to time_t for easier date manipulation
    auto current_time = system_clock::to_time_t(now);

    // Convert to tm struct to get individual time components
    std::tm* local_tm = std::localtime(&current_time);

    // Reset hours, minutes, seconds to get start of day
    local_tm->tm_hour = 0;
    local_tm->tm_min = 0;
    local_tm->tm_sec = 0;

    // Convert back to time_point
    auto start_of_day = system_clock::from_time_t(std::mktime(local_tm));

    // Calculate duration since start of day
    auto duration = now - start_of_day;

    // Convert to milliseconds
    return (uint32_t)duration_cast<milliseconds>(duration).count();
}

PasswordHasher::PasswordHasher()
{

}

String PasswordHasher::getRandomString(int length) const
{
    const String possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = length;
    srand(static_cast<uint32_t>(msecsSinceStartOfDay()));

    String randomString;
    randomString.reserve(randomStringLength);
    for(int i = 0; i < randomStringLength; ++i)
    {
        int index = rand() % possibleCharacters.length();
        char nextChar = possibleCharacters.at(index);
        randomString.push_back(nextChar);
    }
    return randomString;
}

/*!
 * \brief Generates a random salt of length 16.
 * \return A QByteArray of length 16 containing the generated salt.
 */
String PasswordHasher::generateSalt()
{
    return getRandomString(16);
}

/*!
 * \brief Hashes a password with the given salt using the Sha256 algorithm.
 * \param pass The password to be hashed.
 * \param salt The salt to be appended to the password.
 * \return A QByteArray containing the salted and hashed password.
 */
Vector<uint8_t> PasswordHasher::hashPassword(const String &pass, const String &salt)
{
    String pass_array(pass+salt);
    return StringUtils::sha256_buffer(pass_array);
}

//! @}
