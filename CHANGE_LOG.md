# Changelog
All notable changes to this project/module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project/module adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---
## V1.2.1 - 27.09.2023

### Fixing
 - Time duration measurements bug

### Changed
 - Omit main handler period from configuration
 - Added get systick function to configuration  

### Todo
 - Make a entry & exit possible actions to each state
 - State transition allowance & checking

---
## V1.2.0 - 07.09.2023

### Added
 - Asserts at API calls
 - Check for maximum states at init

### Changed
 - Omit main handler period from configuration
 - Added get systick function to configuration  

### Todo
 - Make a entry & exit possible actions to each state
 - State transition allowance & checking

---
## V1.1.0 - 28.08.2023

### Changed
 - Change state time duration to floating point data type 
 - Added detailed describtion of module usage in README.md
 - Removed doxygen as it was replaced by README.md
 - Removed revision.txt as it was replaced by CHANGE_LOG.md

### Todo
 - Make a entry & exit possible actions to each state
 - State transition allowance & checking

---
## V1.0.0 - 01.05.2022

### Added
 - Initial implementation
 - Initialization of FSM as instances
 - Up to 8 states
 - Configuration of FSM as single cfg table input
 - Supports first entry indication
 - Supports getting duration inside state

### Todo
 - State transition allowance & checking

---