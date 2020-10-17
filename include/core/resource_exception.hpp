/**
 * @file resource_exception.hpp
 * @author Gnik Droy
 * @brief File containing exceptions related to resource management.
 *
 */
#pragma once
#include <stdexcept>
#include <string>

/**
 * @brief Class for managing resourceNotFound exceptions.
 */
class ResourceNotFound : public std::runtime_error {
private:
  /** The identifier/name of the resource. */
  std::string name;

public:
  /**
   * @brief Constructs the exception.
   * @param rname The name/identifier of the resource.
   */
  ResourceNotFound(const std::string &rname)
      : std::runtime_error("Resource not found."), name(rname) {}

  /**
   * @brief Constructs the exception.
   * @param rname The name/identifier of the resource.
   * @param msg The custom exception message.
   */
  ResourceNotFound(const std::string &msg, const std::string &rname)
      : std::runtime_error(msg.c_str()), name(rname){};

  /**
   * @brief Returns the resource name.
   * @return The resource name.
   */
  const std::string &getResourceName() const { return name; };
};