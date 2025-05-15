CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(50) UNIQUE NOT NULL,
    password TEXT NOT NULL,
    full_name TEXT NOT NULL,
    is_admin BOOLEAN DEFAULT FALSE,  
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS wallets (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    balance NUMERIC(15,2) DEFAULT 0.00,
    card_number VARCHAR(20) UNIQUE NOT NULL,
    is_active BOOLEAN DEFAULT TRUE,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS accounts (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    account_type VARCHAR(50) NOT NULL DEFAULT 'basic',
    balance NUMERIC(15,2) DEFAULT 0.00,
    interest_rate NUMERIC(5,2) DEFAULT 0.00,
    duration_months INTEGER DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    closed_at TIMESTAMP
);

CREATE TABLE IF NOT EXISTS deposits (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    amount NUMERIC(15,2) NOT NULL,
    interest_rate NUMERIC(5,2) DEFAULT 0.0,
    duration_months INTEGER DEFAULT 0,
    is_active BOOLEAN DEFAULT TRUE,
    opened_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    closed_at TIMESTAMP
);

CREATE TABLE IF NOT EXISTS transactions (
    id SERIAL PRIMARY KEY,
    sender_account_id INTEGER REFERENCES accounts(id) ON DELETE SET NULL,
    receiver_account_id INTEGER REFERENCES accounts(id) ON DELETE SET NULL,
    sender_wallet_id INTEGER REFERENCES wallets(id) ON DELETE SET NULL,
    receiver_wallet_id INTEGER REFERENCES wallets(id) ON DELETE SET NULL,
    sender_deposit_id INTEGER REFERENCES deposits(id) ON DELETE SET NULL,
    receiver_deposit_id INTEGER REFERENCES deposits(id) ON DELETE SET NULL,
    type VARCHAR(50) NOT NULL,
    amount NUMERIC(15,2) NOT NULL CHECK (amount > 0),
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);